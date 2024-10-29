#include "main.hpp"

int main(int argc, char* argv[])
{
	// Initialize graphics and output console
	gfxInitDefault();
	gfxSet3D(true);
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

	// Initialize networking
	u32* SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if (SOC_buffer == NULL)
	{
		std::cout << "Failed to allocate SOC buffer" << std::endl;
		return 1;
	}

	if ((socInitRet = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0)
	{
		std::cout << "socInit failed...womp womp" << std::endl;
		return 1;
	}

	curl_global_init(CURL_GLOBAL_SSL);

	Core core;
	while (aptMainLoop())
	{
		if (hidKeysDown() & KEY_START)
			break; // break in order to return to hbmenu
		
		core.Update();
		core.Render();
	}

	socExit();
	curl_global_cleanup();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}