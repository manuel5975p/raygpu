import pyraygpu as r

r.init_window(1280, 720, "Python Window")

img = r.gen_image_checker(r.GREEN, r.BLACK, 400, 400, 5)

tex = r.load_texture_from_image(img)

while not r.window_should_close():
    
    r.begin_drawing()
    r.clear_background(r.RED)
    r.draw_rectangle(200, 200, 200, 200, r.BLUE)
    r.begin(r.triangles)
    r.color3f(1,0,0)
    r.vertex2f(100.0, 100.0)
    r.color3f(0,1,0)
    r.vertex2f(100.0, 200.0)
    r.color3f(0,0,1)
    r.vertex2f(200.0, 100.0)
    r.end()
    r.draw_texture(tex, 400, 200)
    r.draw_fps()
    r.end_drawing()
