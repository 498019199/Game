add_library(ZENGINE_Render_D3D11 SHARED
	src/D3D11Adapter.cpp
	src/D3D11Adapter.h
	src/D3D11AdapterList.cpp
	src/D3D11AdapterList.h
	#src/D3D11Fence.cpp
	#src/D3D11Fence.h
	#src/D3D11FrameBuffer.cpp
	#src/D3D11FrameBuffer.h
	src/D3D11GraphicsBuffer.cpp
	src/D3D11GraphicsBuffer.h
	#src/D3D11Query.cpp
	#src/D3D11Query.h
	src/D3D11RenderEngine.cpp
	src/D3D11RenderEngine.h
	src/D3D11RenderFactory.cpp
	src/D3D11RenderFactory.h
	src/D3D11RenderLayout.cpp
	src/D3D11RenderLayout.h
	src/D3D11RenderStateObject.cpp
	src/D3D11RenderStateObject.h
	src/D3D11RenderView.cpp
	src/D3D11RenderView.h
	#src/D3D11RenderWindow.cpp
	#src/D3D11RenderWindow.h
	src/D3D11ShaderObject.cpp
	src/D3D11ShaderObject.h
	src/D3D11Texture.cpp
	src/D3D11Texture.h
	#src/D3D11Texture1D.cpp
	src/D3D11Texture2D.cpp
	#src/D3D11Texture3D.cpp
	#src/D3D11TextureCube.cpp
	src/D3D11Util.cpp
	src/D3D11Util.h)
	#src/D3D11VideoMode.cpp
	#src/D3D11VideoMode.h)

target_precompile_headers(ZENGINE_Render_D3D11
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/Context.h")

target_include_directories(ZENGINE_Render_D3D11
	PUBLIC
		${ZENGINE_ROOT_HEADER}/include
		${ZENGINE_ROOT_DIR}/3rd/dxsdk/dxsdk/include
)

set_target_properties(ZENGINE_Render_D3D11 PROPERTIES
		OUTPUT_NAME ZENGINE_Render_D3D11${ZENGINE_OUTPUT_SUFFIX}
		FOLDER "ZENGINE"
	)