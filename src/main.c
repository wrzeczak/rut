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

#define FIX_ATTEMPT_MAX 10000

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
	return (Vector2) { floorf(vec.x / CELLSIZE), floorf(vec.y / CELLSIZE) };
}

// return the grid index of a point
int grid_index(Vector2 vec, int grid_width, int grid_height) {
	Vector2 v = grid_position(vec);
	(void) grid_height; // wasn't technically needed, but included for whatever reason

	return v.x + (v.y * grid_width);
}

//------------------------------------------------------------------------------

pvector generate_random_points(int grid_width, int grid_height) {
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

					for(int j = 0; j < FIX_ATTEMPT_MAX; j++) {
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

int main(void) {

	r = CELLSIZE * sqrt(n); // as previously defined
	pvector poisson_points = generate_random_points(WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
	int poisson_point_count = pvector_size(&poisson_points);

	printf("Poisson points are finished generating!\n");

	InitWindow(WIDTH, HEIGHT, "The Game of all time");
	SetTargetFPS(150);

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);

	while(!WindowShouldClose()) {

		if(IsKeyPressed(KEY_SPACE)) {
			pvector_clear(&poisson_points);
			poisson_points = generate_poisson_points(WIDTH / CELLSIZE, HEIGHT / CELLSIZE);
			poisson_point_count = pvector_size(&poisson_points);
			printf("Poisson points regenerated!\n");
		}

		BeginTextureMode(target);
			ClearBackground(BLACK);

			for(int i = 0; i < poisson_point_count; i++) {
				Vector2 px = poisson_points.data[i];
				if(px.x != -1 && px.y != -1) {
					DrawPixelV(px, RED);
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

