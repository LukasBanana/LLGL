ChangeLog v0.02
===============

`ShaderProgram` interface:
--------------------------

Before:
```cpp
BuildInputLayout(const VertexFormat& vertexFormat);
```

After:
```cpp
BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats);
```

Transition:
```cpp
LLGL::VertexFormat myVertexFormat;
/* ... */
myShaderProgram->BuildInputLayout(1, &myVertexFormat);
```

