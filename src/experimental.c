#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

//------------------------------------------------------------------------------

const int HEIGHT = 900;
const int WIDTH = 1600;
const int CELLSIZE = 20;

#define NOSAMPLE (Vector2) { -1, -1 }

#define ACTIVE_LIST_MAX 50000
#define TRIANGULATION_ITER_MAX 5000

const int n = 2;
float r; // such that the cell size (GRIDSIZE) can be 20
int k = 30; // poisson generation depth

//------------------------------------------------------------------------------

// returns zero if the two vectors are equal
int Vector2_cmp(const Vector2 * a, const Vector2 * b) {
	int c = ( a->x - b->x ) + ( a->y - b->y ); // if the two vectors are equal, this should be zero
	return c;
}

void Vector2_drop(Vector2 * self) {
	// pretty sure nothing needs to be done here...
	(void) self;
}

// clones a Vector2
Vector2 Vector2_clone(Vector2 vec) {
	Vector2 o = (Vector2) { vec.x, vec.y };
	return o;
}

// pvector = point vector, stores Vector2
#define i_type pvector
#define i_keyclass Vector2
#include "stc/cvec.h"
// github.com/stclib/STC/blob/master/docs/cvec_api.md

#define i_type ivector
#define i_key int
#include "stc/cvec.h"

//------------------------------------------------------------------------------

// Vector2 and pvector utility functions

// generate a random point { ( 0 to domain_x_max ), ( 0 to domain_y_max ) }
Vector2 random_point(int domain_x_max, int domain_y_max) {
	return (Vector2) { (int) rand() % domain_x_max, (int) rand() % domain_y_max };
}

// return a vector of the grid position of a point
Vector2 grid_position(Vector2 vec) {
	return (Vector2) { floorf(vec.x / CELLSIZE), floorf(vec.y / CELLSIZE) };
}

// return the grid index of a point
int grid_index(Vector2 vec, int grid_width, int grid_height) {
	Vector2 v = grid_position(vec);
	(void) grid_height; // wasn't technically needed, but included for whatever reason

	return v.x + (v.y * grid_width);
}

float distance(Vector2 a, Vector2 b) {
	return sqrtf(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

float index_distance(pvector * grid, int a, int b) {
	Vector2 va = *(pvector_at(grid, a));
	Vector2 vb = *(pvector_at(grid, b));
	return distance(va, vb);
}

//------------------------------------------------------------------------------

pvector generate_poisson_points(int grid_width, int grid_height) {
	pvector grid = { 0 };

	int grid_size = grid_width * grid_height;

	// generate a bunch of random points inside each cell
	for(int i = 0; i < (int) grid_size; i++) {
		Vector2 inline_position = (Vector2) { rand() % CELLSIZE, rand() % CELLSIZE };
		int grid_x = i % grid_width;
		int grid_y = (i - (i % grid_width)) / grid_height;

		Vector2 position = (Vector2) { inline_position.x + (grid_x * CELLSIZE), inline_position.y + (grid_y * CELLSIZE) };
		pvector_push(&grid, position);
	}

	for(int i = 0; i < grid_size; i++) {
		Vector2 position = *(pvector_at(&grid, i));

		for(int gx = -1; gx < 2; gx++) {
			for(int gy = -1; gy < 2; gy++) {
				int i_translate = gx + (gy * grid_width);
				bool position_fixed = false;
				if(i + i_translate >= 0 && (i + i_translate) < grid_size) {
					Vector2 neighbor = *(pvector_at(&grid, i + i_translate));

					for(int j = 0; j < 20; j++) {
						float distance = sqrtf(pow(neighbor.x - position.x, 2) + pow(neighbor.y - position.y, 2));

						if(distance < r) {
							Vector2 inline_position = (Vector2) { rand() % CELLSIZE, rand() % CELLSIZE };
							int grid_x = i % grid_width;
							int grid_y = (i - (i % grid_width)) / grid_height;

							position = (Vector2) { inline_position.x + (grid_x * CELLSIZE), inline_position.y + (grid_y * CELLSIZE) };
						} else {
							position_fixed = true;
						}
					}

					if(position_fixed)
						grid.data[i] = position;
					else
						grid.data[i] = NOSAMPLE;
				}
			}
		}
	}

	return grid;
}

//------------------------------------------------------------------------------

// return 2 * pvector_size(&points) if it for whatever reason fails
unsigned int closest_to(pvector points, unsigned int excluded_indices[], unsigned int excluded_index_count,  unsigned int index, int grid_width, int grid_height) {
	Vector2 testing_point = *(pvector_at(&points, index));

	float shortest_distance = sqrtf((float) ( HEIGHT * HEIGHT + WIDTH * WIDTH ));
	unsigned int shortest_distance_index = 2 * (int) pvector_size(&points);

	for(int gx = -1; gx < 2; gx++) {
		for(int gy = -1; gy < 2; gy++) {
			int neighbor_offset = gx + (gy * grid_width);

			bool using_excluded_value = false;
			for(int i = 0; i < (int) excluded_index_count; i++) {
				if(((int) index + neighbor_offset) == excluded_indices[i]) using_excluded_value = true;
			}

			if(!using_excluded_value) {

				if(((int) index + neighbor_offset) >= 0 && ((int) index + neighbor_offset) < (grid_width * grid_height)) {
					Vector2 neighbor = *(pvector_at(&points, ((int) index + neighbor_offset)));

					float neighbor_distance = distance(neighbor, testing_point);

					if(neighbor_distance < shortest_distance) {
						shortest_distance = neighbor_distance;
						shortest_distance_index = (int) index + neighbor_offset;
					}
				}
			}
		}
	}

	return shortest_distance_index;
}


// returns a sorted pvector -- every 3 points is a triangle
ivector triangulate_points(pvector points, int grid_width, int grid_height) {
	unsigned int point_count = (unsigned int) pvector_size(&points);
	unsigned int starting_index = rand() % point_count;

	ivector indicies = { 0 };

	// manually populate the first three elements
	ivector_push(&indicies, starting_index);

	ivector(&indicies, closest_to(points, used_indicies, used_index_count, used_indicies[used_index_count - 1], grid_width, grid_height));

	used_indicies[used_index_count] = closest_to(points, used_indicies, used_index_count, used_indicies[used_index_count - 1], grid_width, grid_height);
	ivector(&indicies, used_indicies[used_index_count]);
	used_index_count++;

	pvector_push(&output, *(pvector_at(&points, used_indicies[0])));
	pvector_push(&output, *(pvector_at(&points, used_indicies[1])));
	pvector_push(&output, *(pvector_at(&points, used_indicies[2])));

	unsigned int iter = 0;
	while(used_index_count < (point_count - 1) && iter < TRIANGULATION_ITER_MAX) {

		unsigned int next_index = closest_to(points, used_indicies, used_index_count, (int) *(ivector_at(&indicies, (int) ivector_size(&indicies) - 1)), grid_width, grid_height);

		float dist_a = index_distance(&points, next_index, (int) *(ivector_at(&indicies, (int) ivector_size(&indicies) - 1)));
		float dist_b = index_distance(&points, next_index, (int) *(ivector_at(&indicies, (int) ivector_size(&indicies) - 2)));
		float dist_c = index_distance(&points, next_index, (int) *(ivector_at(&indicies, (int) ivector_size(&indicies) - 3)));

		float smallest_distance = dist_a;

		if(dist_b < smallest_distance) smallest_distance = dist_b;
		if(dist_c < smallest_distance) smallest_distance = dist_c;

		float largest_distance = dist_a;

		if(dist_b > largest_distance) largest_distance = dist_b;
		if(dist_c > largest_distance) largest_distance = dist_c;

		float middle_distance = dist_a;

		if(dist_b != largest_distance && dist_b != smallest_distance) middle_distance = dist_b;
		if(dist_c != largest_distance && dist_c != smallest_distance) middle_distance = dist_c;

		// next triangle is { next_index, smallest_distance, middle_distance };

		ivector_push(&indicies, next_index);
		ivector_push(&indicies, largest_distance);
		ivector_push(&indicies, smallest_distance);

		iter++;
	}

	return indicies;
}


int main(void) {

	r = CELLSIZE * sqrt(n); // as previously defined
	pvector poisson_points = generate_poisson_points(WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
	int poisson_point_count = pvector_size(&poisson_points);

	printf("Poisson points are finished generating!\n");

	ivector triangulated_points = triangulate_points(poisson_points, WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
	int triangle_point_count = ivector_size(&triangulated_points);

	printf("Triangle points are finished generating!\n");

	InitWindow(WIDTH, HEIGHT, "The Game of all time");
	SetTargetFPS(150);

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

	while(!WindowShouldClose()) {

		if(IsKeyPressed(KEY_SPACE)) {
			pvector_clear(&poisson_points);
			poisson_points = generate_poisson_points(WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
			poisson_point_count = pvector_size(&poisson_points);
			printf("Poisson points regenerated!\n");

			triangulated_points = triangulate_points(poisson_points, WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
			triangle_point_count = ivector_size(&triangulated_points);

			printf("Triangle points are finished generating!\n");
		}

		BeginTextureMode(target);
			ClearBackground(BLACK);

			for(int i = 0; i < poisson_point_count; i++) {
				Vector2 px = poisson_points.data[i];
				if(px.x != -1 && px.y != -1) {
					DrawPixelV(px, RED);
				}
			}

			for(int i = 0; i < triangle_point_count; i++) {
				Vector2 a = poisson_points.data[*(ivector_at(&triangulated_points, i))];
				Vector2 b = poisson_points.data[*(ivector_at(&triangulated_points, i + 1))];
				Vector2 c = poisson_points.data[*(ivector_at(&triangulated_points, i + 2))];

				DrawTriangleLines(a, b, c, RAYWHITE);
			}

		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			DrawTexture(target.texture, 0, 0, WHITE);

			DrawFPS(10, 10);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}

