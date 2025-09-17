file(REMOVE_RECURSE
  "libraygpu.a"
  "libraygpu.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C CXX)
  include(CMakeFiles/raygpu.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
