# Backend include files

Use these include files (`*.inl`) to declare all required functions for the respective interface for your own renderer implementation. None of these headers are required for regular use of LLGL.

## Example

### `MyOwnRenderer.h`

```cpp
#include <LLGL/CommandBuffer.h>

class MyOwnRenderer : public LLGL::CommandBuffer {
public:
    #include <LLGL/Backend/CommandBuffer.inl>
public:
    MyOwnRenderer();
    ~MyOwnRenderer();
private:
    MyRendererData privateData;
};
```

### `MyOwnRenderer.cpp`

```cpp
#include "MyOwnRenderer.h"

MyOwnRenderer::MyOwnRenderer() {
    /* Initialize OpenGL or Direct3D etc. */
}
MyOwnRenderer::~MyOwnRenderer() {
    /* Clean up OpenGL or Direct3D etc. */
}
void MyOwnRenderer::Begin() {
    /* Implements LLGL::CommandBuffer::Begin() ... */
}
void MyOwnRenderer::End() {
    /* Implements LLGL::CommandBuffer::End() ... */
}

/* All other interface functions ... */
```
