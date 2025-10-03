#include "PGR/Application.h"

int main(int argc, char** argv) {
	PGR::Application App(argc, argv, "PGR", 640, 480);

	App.Run();

	return 0;
}
