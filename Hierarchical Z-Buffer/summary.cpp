#include <cstdio>
#include "procedure.h"

void summary()
{
	printf("********                SUMMARY               ********\n\n");
	
	printf("None Z-Buffer\n");
	printf("RUNTIME: %fs\n", timeN);
	printf("FRAMES:  %d\n", frameN);
	printf("FPS:     %f\n\n", frameN / timeN);

	printf("Scanline Z-Buffer\n");
	printf("RUNTIME: %fs\n", timeS);
	printf("FRAMES:  %d\n", frameS);
	printf("FPS:     %f\n\n", frameS / timeS);

	printf("Baseline Hierarchial Z-Buffer\n");
	printf("RUNTIME: %fs\n", timeB);
	printf("FRAMES:  %d\n", frameB);
	printf("FPS:     %f\n\n", frameB / timeB);

	printf("Hierarchial Z-Buffer\n");
	printf("RUNTIME: %fs\n", timeH);
	printf("FRAMES:  %d\n", frameH);
	printf("FPS:     %f\n\n", frameH / timeH);
		
	printf("******************************************************\n");
}