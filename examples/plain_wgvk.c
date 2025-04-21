#include <wgvk.h>
void TraceLog(int logType, const char *text, ...){
    
}
int main(){
    WGVKInstanceDescriptor instanceDescriptor = {
        .capabilities = {},
        .nextInChain = NULL
    };
    wgvkCreateInstance(&instanceDescriptor);
}