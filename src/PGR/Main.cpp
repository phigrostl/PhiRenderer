#include "PGR/Application.h"

int main(int argc, char** argv) {

	if (_chdir("../../resources"))
		exit(1);

	PGR::Application App(argc, argv, "PGR", 640, 360);

	App.Run();

	return 0;
}
