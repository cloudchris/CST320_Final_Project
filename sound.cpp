
#include "sound.h"



int run_mp3_thread(audio_args_ *ccall);
DWORD WINAPI runThread_and_delete(LPVOID args)
	{
	audio_args_ *ccall = (audio_args_*)(args);
	run_mp3_thread(ccall);
	delete ccall->file;
	delete ccall;
	return 0;
	}
DWORD WINAPI runThread(LPVOID args)
	{
	audio_args_ *ccall = (audio_args_*)(args);
	run_mp3_thread(ccall);
	return 0;
	}
void start_music(LPWSTR file, int volume)
	{
	DWORD threadId;
	audio_args_ *audio_args = new audio_args_;
	audio_args->file = file;
	audio_args->volume = volume;
	CreateThread(NULL, 0, runThread_and_delete, audio_args, 0, &threadId);
	}
void start_music(audio_args_ *audio_args)
	{
	DWORD threadId;
	CreateThread(NULL, 0, runThread, audio_args, 0, &threadId);
	}

int run_mp3_thread(audio_args_ *audio_args)
	{
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;
	

	// Initialize the COM library.
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
		{
		::printf("ERROR - Could not initialize COM library");
		return 0;
		}

	// Create the filter graph manager and query for interfaces.
	hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr))
		{
		::printf("ERROR - Could not create the Filter Graph Manager.");
		return 0;
		}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr = pGraph->QueryInterface(IID_IBasicAudio, (LPVOID *)&audio_args->pBasicAudio);

	// Build the graph.
	hr = pGraph->RenderFile(audio_args->file, NULL);

	audio_args->pBasicAudio->put_Volume(audio_args->volume);
	
	if (SUCCEEDED(hr))
		{
		// Run the graph.
		hr = pControl->Run();

		if (SUCCEEDED(hr))
			{
			// Wait for completion.
			long evCode;
			pEvent->WaitForCompletion(1000000, &evCode);

			// Note: Do not use INFINITE in a real application, because it
			// can block indefinitely.
			}
		}
	// Clean up in reverse order.
	audio_args->pBasicAudio->Release();
	audio_args->pBasicAudio = NULL;
	pEvent->Release();
	pControl->Release();
	pGraph->Release();
	::CoUninitialize();
	}

wchar_t *GetWC(char *c)
	{
	/*
	wchar_t wc[1000];
	mbstowcs(wc, c, strlen(c)+1);
	return wc;
	*/

	size_t arrSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[arrSize];
	mbstowcs(wc, c, arrSize);
	return wc;
	}