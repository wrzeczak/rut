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

//------------------------------------------------------------------------------

// Vector2 and pvector utility functions

// generate a random point { ( 0 to domain_x_max ), ( 0 to domain_y_max ) }
Vector2 random_point(int domain_x_max, int domain_y_max) {
	return (Vector2) { (int) rand() % domain_x_max, (int) rand() % domain_y_max };
}

// return a vector of the grid position of a point
Vector2 grid_position(Vector2 vec) {
	return (Vector2) { floor(vec.x / CELLSIZE), floor(vec.y / CELLSIZE) };
}

// return the grid index of a point
int grid_index(Vector2 vec, int grid_width, int grid_height) {
	Vector2 v = grid_position(vec);

	return v.x + (v.y * grid_width);
}

//------------------------------------------------------------------------------

pvector generate_poisson_points(int grid_width, int grid_height) {
	pvector grid = { 0 };
	int grid_size = grid_width * grid_height;

	// initalize NOSAMPLE in every grid space
	for(int i = 0; i < grid_size; i++) {
		pvector_push(&grid, NOSAMPLE);
	}

	srand(time(NULL));
	Vector2 initial_sample = random_point(WIDTH, HEIGHT);
	grid.data[grid_index(initial_sample, grid_width, grid_height)] = initial_sample;

	pvector active_list = { 0 };
	pvector_push(&active_list, initial_sample);

	while(((int) pvector_size(&active_list) > 0)) {

		printf("Made it to the while() loop!\n");

		int active_list_length = (int) pvector_size(&active_list);

		printf("ACTIVE LIST LENGTH: %d\n", active_list_length);

		int random_index = rand() % active_list_length;

		Vector2 xi = active_list.data[random_index]; // select a random point from the active list

		int failed_candidates = 0;
		for(int i = 0; i < k; i++) {
			float magnitude = ((float) rand() / (float) RAND_MAX) * r + r;
			float theta = ((float) rand() / (float) RAND_MAX) * (2 * PI);

			Vector2 candidate = (Vector2) { (int) floor(magnitude * cos(theta)) + xi.x, (int) floor(magnitude * sin(theta)) + xi.y};
			int candidate_index = grid_index(candidate, grid_width, grid_height);
			Vector2 candidate_grid_position = grid_position(candidate);

			for(int gy = -2; gy < 3; gy++) {
				for(int gx = -2; gx < 3; gx++) {
					int neighbor_index = candidate_index + gx + (gy * grid_width);

					Vector2 neighbor = grid.data[neighbor_index];

					float distance = sqrt(pow(candidate.x - neighbor.x, 2) + pow(candidate.y - neighbor.y, 2));

					if((neighbor.x == -1 && neighbor.y == -1) || (distance > r)) {
						// this cell is good!
						// there is nothing to do except wait...
					} else {
						// this candidate is no good
						goto point_failed;
					}
				}
			}

			// point succeeded
			pvector_push(&active_list, candidate);
			grid.data[candidate_index] = candidate;
			point_failed:
				failed_candidates++;
		}

		// remove xi+
		pvector_erase_n(&active_list, random_index, 1);
	}

	return grid;
}

//------------------------------------------------------------------------------

#define i_implement
#include "stc/cstr.h"

int main(void) {

	r = CELLSIZE * sqrt(n); // as previously defined
	pvector poisson_points_pvector = generate_poisson_points(WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
	int poisson_point_count = pvector_size(&poisson_points_pvector);

	Vector2 poisson_points[poisson_point_count];

	// convert to C array because pvector_at is pretty slow compared to array access
	for(int i = 0; i < poisson_point_count; i++) {
		poisson_points[i] = *(pvector_at(&poisson_points_pvector, (intptr_t) i));
	}


	printf("Poisson points are finished generating!\n");

	InitWindow(WIDTH, HEIGHT, "The Game of all time");
	SetTargetFPS(150);

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

	while(!WindowShouldClose()) {
		BeginTextureMode(target);

			for(int i = 0; i < poisson_point_count; i++) {
				Vector2 px = poisson_points[i];
				if(px.x != -1 && px.y != -1) {
					DrawCircleV(px, 10, RED);
				}
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

