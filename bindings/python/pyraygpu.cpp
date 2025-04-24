#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For vector, list conversions
#include <pybind11/operators.h> // For comparison operators on structs (will use lambdas instead for safety)
#include <pybind11/numpy.h> // For comparison operators on structs (will use lambdas instead for safety)
#include <raygpu.h>

#include <vector>
#include <string>
#include <cmath> // For roundf

namespace py = pybind11;

// Helper to wrap the opaque cwoid handle
class PyRaygpuWindow {
public:
    void* handle;

    PyRaygpuWindow(void* h) : handle(h) {
        if (!handle) {
            throw std::runtime_error("Failed to create raygpu window handle (returned NULL).");
        }
    }

    // No explicit destructor for the window handle itself.
    // The lifecycle is likely tied to SetWindowShouldClose and the main loop exiting.
    // raygpu.h doesn't expose a explicit CloseWindow(cwoid).

    // Delete copy constructor and assignment operator
    PyRaygpuWindow(const PyRaygpuWindow&) = delete;
    PyRaygpuWindow& operator=(const PyRaygpuWindow&) = delete;

    // Allow move operations
    PyRaygpuWindow(PyRaygpuWindow&& other) noexcept : handle(other.handle) {
        other.handle = nullptr; // Prevent other from invalidating handle
    }

    PyRaygpuWindow& operator=(PyRaygpuWindow&& other) noexcept {
        if (this != &other) {
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
};


// Helper to manage resources with Load/Unload pattern
// This version handles resource types that are passed by value to the Unload function.
template <typename T, void (*unload_func)(T)>
class ResourceWrapperValue {
public:
    T resource;
    bool valid;

    ResourceWrapperValue() : valid(false) {} // Default invalid state

    // Constructor that takes an already loaded resource
    ResourceWrapperValue(T res) : resource(res), valid(true) {
        // Add basic validity checks for common types if possible
        if constexpr (std::is_same_v<T, Texture>) {
            if (resource.id == 0) valid = false;
        }
        if constexpr (std::is_same_v<T, Image>) {
             // A loaded image should have data, width, and height set
             if (resource.data == nullptr && resource.width > 0 && resource.height > 0) valid = false;
             if (resource.width == 0 || resource.height == 0) valid = false;
        }
        // Add checks for other types if needed
        if (!valid) {
             // Clear the resource data to ensure it's truly invalid
             if constexpr (std::is_same_v<T, Texture>) { resource = {}; } // Zero-initialize struct
             if constexpr (std::is_same_v<T, Image>) { resource = {}; } // Zero-initialize struct
        }
    }

    // Custom destructor to call the unload function
    ~ResourceWrapperValue() {
        if (valid) {
            unload_func(resource);
            valid = false; // Mark as invalid after unloading
        }
    }

    // Copy constructor and assignment operator deleted to prevent double free
    ResourceWrapperValue(const ResourceWrapperValue&) = delete;
    ResourceWrapperValue& operator=(const ResourceWrapperValue&) = delete;

    // Move constructor and assignment operator
    ResourceWrapperValue(ResourceWrapperValue&& other) noexcept
        : resource(other.resource), valid(other.valid) {
        other.valid = false; // Prevent other from unloading
         // Clear other's resource data after moving
        if constexpr (std::is_same_v<T, Texture>) { other.resource = {}; }
         if constexpr (std::is_same_v<T, Image>) { other.resource = {}; }
    }

    ResourceWrapperValue& operator=(ResourceWrapperValue&& other) noexcept {
        if (this != &other) {
            if (valid) {
                unload_func(resource); // Cleanup self
            }
            resource = other.resource;
            valid = other.valid;
            other.valid = false; // Prevent other from unloading
             // Clear other's resource data
            if constexpr (std::is_same_v<T, Texture>) { other.resource = {}; }
             if constexpr (std::is_same_v<T, Image>) { other.resource = {}; }
        }
        return *this;
    }

    // Allow implicit conversion to the underlying type T
    // This allows passing the wrapper directly to C++ functions expecting T
    operator T() const {
        if (!valid) {
            // Throw an exception to indicate invalid resource usage
            throw std::runtime_error("Attempted to use invalid raygpu resource.");
        }
        return resource;
    }

    // Expose the underlying resource by reference
    // This allows accessing/modifying members of the wrapped struct from Python
    T& get() {
        if (!valid) throw std::runtime_error("Attempted to get reference to invalid raygpu resource.");
        return resource;
    }
    const T& get() const {
        if (!valid) throw std::runtime_error("Attempted to get const reference to invalid raygpu resource.");
        return resource;
    }

    bool is_valid() const { return valid; }
};

// Helper for Image data access - returns a copy
py::bytes image_get_data_copy(const Image& img) {
    if (!img.data || img.width == 0 || img.height == 0 || img.rowStrideInBytes == 0 || GetPixelSizeInBytes(img.format) == 0) {
        return py::bytes(""); // Return empty bytes for invalid/empty/unsupported image
    }

    size_t pixel_size = GetPixelSizeInBytes(img.format);
    size_t expected_row_size = img.width * pixel_size;
    size_t copy_size = img.height * expected_row_size; // Total size of actual pixel data

    // If rowStrideInBytes is different, we need to copy row by row
    if (img.rowStrideInBytes != expected_row_size) {
        std::string data_copy(copy_size, '\0'); // Allocate space
        const unsigned char* src_ptr = static_cast<const unsigned char*>(img.data);
        char* dst_ptr = data_copy.data();

        for (uint32_t y = 0; y < img.height; ++y) {
            memcpy(dst_ptr + y * expected_row_size, src_ptr + y * img.rowStrideInBytes, expected_row_size);
        }
         return py::bytes(data_copy.data(), data_copy.size()); // Return the copied bytes
    } else {
        // Row stride matches, simple copy
        return py::bytes(static_cast<const char*>(img.data), copy_size);
    }
}
Vector3 Vector3Scale_pb(const Vector3& v, float s) {
    return Vector3{v.x * s, v.y * s, v.z * s};
}
Vector3 Vector3cwp(const Vector3& v, const Vector3& v2) {
    return Vector3{v.x * v2.x, v.y * v2.y, v.z * v2.z};
}
struct image_deleter{
    void operator()(Image* img)const noexcept{
        UnloadImage(*img);
    }
};
PYBIND11_MODULE(pyraygpu, m) {
    m.doc() = "Minimal Pybind11 wrapper for raygpu library";

    
    py::class_<Color>(m, "Color")
        .def(py::init<uint8_t, uint8_t, uint8_t, uint8_t>(), py::arg("r") = 0, py::arg("g") = 0, py::arg("b") = 0, py::arg("a") = 255)
        .def_readwrite("r", &Color::r)
        .def_readwrite("g", &Color::g)
        .def_readwrite("b", &Color::b)
        .def_readwrite("a", &Color::a)
        .def("__repr__", [](const Color& c) -> py::str { // Explicit return type
            return "<Color r=" + std::to_string(c.r) + ", g=" + std::to_string(c.g) + ", b=" + std::to_string(c.b) + ", a=" + std::to_string(c.a) + ">";
        })
        .def("__eq__", [](const Color& a, const Color& b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; })
        .def("__ne__", [](const Color& a, const Color& b) { return !(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a); });

         

    m.def("draw_fps", DrawFPS, py::arg("x") = 5, py::arg("y") = 5);
    m.def("gen_image_color",   [](Color a, uint32_t width, uint32_t height){return GenImageColor(a, width, height);}, py::arg("color"), py::arg("width"), py::arg("height"), "Generate image: plain color");
    m.def("gen_image_checker", [](Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){return GenImageChecker(a, b, width, height, checkerCount);}, py::arg("color1"), py::arg("color2"), py::arg("width"), py::arg("height"), py::arg("checkerCount"), "Generate image: plain color", py::return_value_policy::reference);
    
    //py::class_<std::unique_ptr<int>>(m, "intpointer")
    //    //.def("value", [](const std::unique_ptr<int>& ptr) -> py::int_ {return *ptr;})
    //;
    
    py::class_<Image, std::unique_ptr<Image, image_deleter>>(m, "Image")
        .def_readonly("data", &Image::data)
        //.def("__del__", [](Image &self, py::args){ 
        //    UnloadImage(self);
        //})
        .def("__enter__", [](Image &self) -> Image& { return self; },
            py::return_value_policy::reference)
        .def("__exit__", [](Image &self, py::args){ 
            //UnloadImage(self);
        });
    ;
    py::class_<Rectangle>(m, "Rectangle")
        .def_readwrite("x",      &Rectangle::x)
        .def_readwrite("y",      &Rectangle::y)
        .def_readwrite("width",  &Rectangle::width)
        .def_readwrite("height", &Rectangle::height)
    ;
    
    py::class_<Texture>(m, "Texture")
        .def_readonly("width", &Texture::width)
        .def_readonly("height", &Texture::height);
    
    m.def("load_texture_from_image", LoadTextureFromImage, py::arg("image"));
    m.def("draw_texture", DrawTexture, py::arg("texture"), py::arg("posX"), py::arg("posY"), py::arg("tint") = WHITE);

    m.attr("LIGHTGRAY")      = LIGHTGRAY; m.attr("GRAY")       = GRAY;      m.attr("DARKGRAY")   = DARKGRAY;
        m.attr("YELLOW")     = YELLOW;    m.attr("GOLD")       = GOLD;      m.attr("ORANGE")     = ORANGE;
        m.attr("PINK")       = PINK;      m.attr("RED")        = RED;       m.attr("MAROON")     = MAROON;
        m.attr("GREEN")      = GREEN;     m.attr("LIME")       = LIME;      m.attr("DARKGREEN")  = DARKGREEN;
        m.attr("SKYBLUE")    = SKYBLUE;   m.attr("BLUE")       = BLUE;      m.attr("DARKBLUE")   = DARKBLUE;
        m.attr("PURPLE")     = PURPLE;    m.attr("VIOLET")     = VIOLET;    m.attr("DARKPURPLE") = DARKPURPLE;
        m.attr("BEIGE")      = BEIGE;     m.attr("BROWN")      = BROWN;     m.attr("DARKBROWN")  = DARKBROWN;
        m.attr("WHITE")      = WHITE;     m.attr("BLACK")      = BLACK;     m.attr("BLANK")      = BLANK;
        m.attr("MAGENTA")    = MAGENTA;   m.attr("RAYWHITE")   = RAYWHITE;
    py::enum_<PrimitiveType>(m, "primitive_type").value("triangles", RL_TRIANGLES).export_values();

    py::class_<Vector2>(m, "Vector2")
        .def(py::init<float, float>(), py::arg("x") = 0.0f, py::arg("y") = 0.0f)
        .def_readwrite("x", &Vector2::x)
        .def_readwrite("y", &Vector2::y)
        .def("repr", [](const Vector2& v) -> py::str {
        return "<Vector2 x=" + std::to_string(v.x) + ", y=" + std::to_string(v.y) + ">";
        })
        .def("add", [](const Vector2& a, const Vector2& b){ return Vector2{a.x + b.x, a.y + b.y}; })
        .def("subtract", [](const Vector2& a, const Vector2& b){ return Vector2{a.x - b.x, a.y - b.y}; })
        .def("mul", Vector2Scale)
    ;

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<float, float, float>(), py::arg("x") = 0.0f, py::arg("y") = 0.0f, py::arg("z") = 0.0f)
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)
        .def("__repr__", [](const Vector3& v) -> py::str { // Explicit return type
            return "<Vector3 x=" + std::to_string(v.x) + ", y=" + std::to_string(v.y) + ", z=" + std::to_string(v.z) + ">";
        })
        .def("__add__", [](const Vector3& a, const Vector3& b){ return Vector3{a.x + b.x, a.y + b.y, a.z + b.z}; })
        .def("__subtract__", [](const Vector3& a, const Vector3& b){ return Vector3{a.x - b.x, a.y - b.y, a.z - b.z}; })
        .def("__mul__", Vector3Scale)
    ;

    py::class_<Vector4>(m, "Vector4")
        .def(py::init<float, float, float, float>(), py::arg("x") = 0.0f, py::arg("y") = 0.0f, py::arg("z") = 0.0f, py::arg("w") = 0.0f)
        .def_readwrite("x", &Vector4::x)
        .def_readwrite("y", &Vector4::y)
        .def_readwrite("z", &Vector4::z)
        .def_readwrite("w", &Vector4::w)
        .def("__repr__", [](const Vector4& v) -> py::str {
            return "<Vector4 x=" + std::to_string(v.x) + ", y=" + std::to_string(v.y) + ", z=" + std::to_string(v.z) + ", w=" + std::to_string(v.w) + ">";
        })
        .def("__add__", [](const Vector4& a, const Vector4& b){ return Vector4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; })
        .def("__subtract__", [](const Vector4& a, const Vector4& b){ return Vector4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}; })
        .def("__mul__", Vector4Scale)
    ;
    py::class_<DescribedBuffer>(m, "DescribedBuffer");
    m.def("GenVertexBuffer", [](const std::vector<float>& data){
        DescribedBuffer* ret = nullptr;
        std::vector<float> vec(data.size());
        //for(const auto& elem : data){
        //    std::cout << elem << "\n";
        //}
        //std::transform(x.begin(), x.end(), vec.begin(), [](const auto& x){
        //    
        //});
        return ret;
    }, py::arg("data"), py::return_value_policy::reference);
    m.def("begin", rlBegin, py::arg("PrimitiveType"));
    m.def("end", rlEnd);

    m.def("vertex2f", rlVertex2f, py::arg("x"), py::arg("y"));
    m.def("vertex3f", rlVertex3f, py::arg("x"), py::arg("y"), py::arg("z"));
    m.def("texCoord2f", rlTexCoord2f, py::arg("u"), py::arg("v"));
    m.def("color3f", rlColor3f, py::arg("r"), py::arg("g"), py::arg("b"));
    m.def("color4f", rlColor4f, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"));
    m.def("color4ub", rlColor4ub, py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a"));
    

    m.def("init_window", &InitWindow);
    m.def("window_should_close", &WindowShouldClose);
    m.def("begin_drawing", &BeginDrawing);
    m.def("end_drawing", &EndDrawing);
    m.def("draw_rectangle", &DrawRectangle);
    m.def("clear_background", &ClearBackground);
}