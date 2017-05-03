#include "color_array.h"
#include "camera.h"
#include "scene.h"
#include "sphere.h"
#include <thread>

///renders the scene using the camera into a rectangle ((0,startIndex), (width, startIndex+dy)) of colorArray
void render(const Scene& scene, const Camera& camera, ColorArray& colorArray, int startIndex, int dy, int numSamples)
{
	Ray tempRay;
	for (int j = startIndex; j < startIndex+dy; ++j)
	{
		for (int i = 0; i <colorArray.width; ++i)
		{
			for (int k = 0; k < numSamples; ++k)
			{
				if (camera.generateRay(( random()+i) / (colorArray.width - 1.0), (colorArray.height - (random()+j)) / (colorArray.height - 1.0), tempRay))
				{
					//clr(i,j).set(0.0,0.0,clamp(0.5+0.5*tempRay.direction.y,0.0,1.0));
					IntersectionInfo info;
					if (scene.intersect(tempRay, info))
						colorArray(i, j) += Vec3(1.0f, 1.0f, 1.0f);
				}
			}
			colorArray(i, j) *= (1.0f / numSamples);
		}
	}
}

int main()
{
	///width and height of the output
	int width = 500; int height = 500;
	///number of threads - use whatever your machine has
	int numThreads = 8;
	///samples per pixel
	int numSamples = 30;

	ColorArray clr(width, height);
	Camera camera;
	camera.enableVignetting = true;
	camera.aspectRatio = float(width) / height;
	camera.apertureRadius = 0.03f;
	float aperture2Radius = 0.13f;
	camera.aperture2RadiusSq = aperture2Radius*aperture2Radius;
	//camera.position.set(0.0f, 0.0f, -10.0f);
	//camera.rotate(0.0f, 0.3f, 0.0f);

	Scene scene;
	int numSpheresX = 10;
	int numSpheresY = 10;
	int spheresDX = 5;
	int spheresDY = 5;
	float offsetX = -spheresDX*(numSpheresX-1) / 2.0;
	float offsetY = -spheresDY*(numSpheresY-1) /2.0;
	for (int j = 0; j < numSpheresY; ++j)
	{
		for (int i = 0; i < numSpheresX; ++i)
		{
			scene.objects.push_back(std::make_unique<Sphere>(Sphere(1.0f, Vec3(offsetX+i*spheresDX , offsetY + j*spheresDY, 27.0f))));
		}
	}

	///distribute workload
	std::vector<std::thread> threads;
	int dy = floor(float(height) / numThreads);
	int leftOver = height%numThreads;

	///render the scene
	for (int i = 0; i < numThreads-1; ++i)
	{
		threads.push_back(std::thread(render, std::ref(scene), std::ref(camera), std::ref(clr), dy*i, dy, numSamples));
	}
	threads.push_back(std::thread(render, std::ref(scene), std::ref(camera), std::ref(clr), dy*(numThreads - 1), dy+leftOver, numSamples));
	for (auto& th : threads)
	{
		th.join();
	}
	///save to file
	clr.saveToPPM("testImage.ppm");
	
	
	return 0;
}