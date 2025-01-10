#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED


#ifndef SUPPORT_WGSL_PARSER
    #define SUPPORT_WGSL_PARSER 1
#endif

#ifndef SUPPORT_GLSL_PARSER
    #define SUPPORT_GLSL_PARSER 0
#endif


// The RENDERBATCH_SIZE is how many instances of the vertex type can be batched at most
// It must be a multiple of 12 to guarantee that RL_LINES, RL_TRIANGLES and RL_QUADS 
// trigger an overflow on a completed shape. 
// Because of that, it needs to be a multiple of both 3 and 4

#ifndef RENDERBATCH_SIZE_MULTIPLIER
    #define RENDERBATCH_SIZE_MULTIPLIER 200
#endif

#define RENDERBATCH_SIZE (RENDERBATCH_SIZE_MULTIPLIER * 12)


#endif // CONFIG_H_INCLUDED
