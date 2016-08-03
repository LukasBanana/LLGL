Low Level Graphics Library (LLGL)
=================================

Overview
--------

- **Version**: 1.00 Alpha
- **License**: [3-Clause BSD License](https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt)


Getting Started
---------------

```cpp
#include <LLGL/LLGL.h>

int main()
{
	// Create a window to render into
	LLGL::WindowDescriptor windowDesc;

	windowDesc.title    = L"LLGL Example";
	windowDesc.visible  = true;
	windowDesc.centered = true;
	windowDesc.width    = 640;
	windowDesc.height   = 480;

	auto window = LLGL::Window::Create(windowDesc);

	// Add keyboard/mouse event listener
	auto input = std::make_shared<LLGL::Input>();
	window->AddEventListener(input);

	//TO BE CONTINUED ...

	// Main loop
	while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
	{
		
		// Draw with OpenGL, or Direct3D, or Vulkan, or whatever ...
		
	}
	
	return 0;
}
```


