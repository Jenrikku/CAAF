#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	string file;
	string name;

	if (argc <= 1) {
		cout << "Model file: ";
		cin >> file;
		cout << endl;
	} else {
		file = argv[1];
	}

	cout << "Actor name: ";
	cin >> name;
	cout << endl;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(file, 0);

	if (!scene) {
		cerr << "Could not read model" << endl;
		return -1;
	}

	for (int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		// mesh.
	}
}