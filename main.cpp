#include "color_array.h"
#include "camera.h"
#include "scene.h"
#include "sphere.h"
#include <thread>
#include "high_precision_timer.h"
#include <iostream>
#include <string>

///renders the scene using the camera into a rectangle ((0,startIndex), (width, startIndex+dy)) of colorArray
void render(const Scene& scene, const Camera& camera, ColorArray& colorArray, int startIndex, int dy, int pixelDivisionsX, int pixelDivisionsY)
{
	std::mt19937 randomEngine{ rd() };

	std::uniform_real_distribution<float> uniformDistribution{ 0.0, 1.0 };

	Ray tempRay;
	for (int j = startIndex; j < startIndex+dy; ++j)
	{
		for (int i = 0; i <colorArray.width; ++i)
		{
			for (int l = 0; l < pixelDivisionsY; ++l)
			{
				for (int k = 0; k < pixelDivisionsX; ++k)
				{

					if (camera.generateRay(randomEngine, uniformDistribution, (i+(k+random())/float(pixelDivisionsX-1)) / (colorArray.width - 1.0), (colorArray.height - ((l+random())/float(pixelDivisionsY-1) + j)) / (colorArray.height - 1.0), tempRay))
					{
						//clr(i,j).set(0.0,0.0,clamp(0.5+0.5*tempRay.direction.y,0.0,1.0));
						IntersectionInfo info;
						if (scene.intersect(tempRay, info))
							colorArray(i, j) += Vec3(100.0f);
					}
				}
			}
			colorArray(i, j) *= (1.0f / (pixelDivisionsX*pixelDivisionsY));
		}
	}
}

int main()
{
	///width and height of the output
	int width = 500; int height = 500;
	///number of threads - use whatever your machine has
	int numThreads = 8;
	///number of x and y division of a pixel - each pixel will be divided into x*y equal rectangles and for each rectangle a sample will be taken
	int pixelDivisionsX = 10;
	int pixelDivisionsY = 10;

	ColorArray clr(width, height);

	

	


	Camera camera;
	camera.apertureRadius = 0.05f;
	camera.aspectRatio = float(width) / height;

	camera.enableVignetting = true;
	camera.setAperture2Radius(0.13f);

	camera.enableImageBasedBokeh = false;
	
	camera.loadBokehImage("bokeh_14.jpg");
	//camera.loadBokehImage("bokehTest2.bmp");
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
			scene.objects.push_back(std::make_unique<Sphere>(Sphere(0.1f, Vec3(offsetX+i*spheresDX , offsetY + j*spheresDY, 27.0f))));
		}
	}

	///distribute workload
	std::vector<std::thread> threads;
	int dy = floor(float(height) / numThreads);
	int leftOver = height%numThreads;

	HighPrecisionTimer timer;
	float startTimeStamp = timer.getTime();
	///render the scene
	for (int i = 0; i < numThreads-1; ++i)
	{
		threads.push_back(std::thread(render, std::ref(scene), std::ref(camera), std::ref(clr), dy*i, dy, pixelDivisionsX, pixelDivisionsY));
	}
	threads.push_back(std::thread(render, std::ref(scene), std::ref(camera), std::ref(clr), dy*(numThreads - 1), dy+leftOver, pixelDivisionsX, pixelDivisionsY));
	for (auto& th : threads)
	{
		th.join();
	}
	float timeTaken = timer.getTime() - startTimeStamp;
	std::cout << "Time: " << timeTaken << "\n";

	///save to file
	clr.saveToPPM("testImage.ppm");
	
	
	
	return 0;
}