#define CATCH_CONFIG_RUNNER
#include <QtGui/QGuiApplication>
#include <catch2/catch.hpp>

int main(int argc, char** argv)
{
	QGuiApplication app(argc, argv);
	return Catch::Session().run(argc, argv);
}
