#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

//------------------------------------------------------------------------------

const int HEIGHT = 900;
const int WIDTH = 1600;
const int CELLSIZE = 10;

const int MAPWIDTH = WIDTH / CELLSIZE;
const int MAPHEIGHT = HEIGHT / CELLSIZE;

#include "country_colors.h"
const int COUNTRYMAX = COLOR_REGISTER_LENGTH;

#define NOSAMPLE (Vector2) { -1, -1 }

//------------------------------------------------------------------------------

typedef struct {
	int x, y;
	Color c;
} Point;

//------------------------------------------------------------------------------

// returns zero if the two vectors are equal
int Point_cmp(const Point * a, const Point * b) {
	return (a->y == b->y && a->x == b->x) ? true : false;
}

void Point_drop(Point * self) {
	// pretty sure nothing needs to be done here...
	(void) self;
}

// clones a Vector2
Point Point_clone(Point vec) {
	Point o = (Point) { vec.x, vec.y, vec.c };
	return o;
}

// pvector = point vector, stores Point
#define i_type pvector
#define i_keyclass Point
#include "stc/cvec.h"
// github.com/stclib/STC/blob/master/docs/cvec_api.md

//------------------------------------------------------------------------------

float distance(Point a, Point b) {
	return sqrtf((float) (pow(a.x - b.x, 2) + pow(a.y - b.y, 2)));
}

//------------------------------------------------------------------------------

pvector generate_starting_countries() {
	// int country_count = 16;
	int country_grid_x = 8;
	int country_grid_y = 2;
	int grid_x_unit = floor(MAPWIDTH / country_grid_x);
	int grid_y_unit = floor(MAPHEIGHT / country_grid_y);

	// country_grid_y * country_grid_x should equal country_count

	pvector output = { 0 };

	pvector starting_points = { 0 };

	// put a starting point in each grid cell
	for(int gy = 0; gy < country_grid_y; gy++) {
		for(int gx = 0; gx < country_grid_x; gx++) {
			int x_offset = gx * grid_x_unit;
			int y_offset = gy * grid_y_unit;

			int x = rand() % grid_x_unit;
			int y = rand() % grid_y_unit;

			x += x_offset;
			y += y_offset;

			Point pnt = (Point) { x, y, country_color(gy * country_grid_x + gx) };

			pvector_push(&starting_points, pnt);
		}
	}

	// seeds points should be relatively evenly spaced out while still having countries of varying sizes

	for(int i = 0; i < (int) pvector_size(&starting_points); i++) {
		pvector_push(&output, *(pvector_at(&starting_points, i)));
	}

	for(int y = 0; y < MAPHEIGHT; y++) {
		for(int x = 0; x < MAPWIDTH; x++) {
			Point pnt = (Point) { x, y, country_color(-1) };

			float shortest_distance = distance((Point) { 0, 0, country_color(-1) }, (Point) { MAPWIDTH, MAPHEIGHT, country_color(-1) });

			for(int j = 0; j < (int) pvector_size(&starting_points); j++) {

				Point ref = *(pvector_at(&starting_points, j));
				float d = distance(pnt, ref);

				if(d < shortest_distance) {
					pnt.c = country_color(j);
					shortest_distance = d;
				}
			}

			pvector_push(&output, pnt);
		}
	}

	pvector sorted_output = { 0 };

	// comically inefficient, but it'll get the job done for now
	for(int y = 0; y < MAPHEIGHT; y++) {
		for(int x = 0; x < MAPWIDTH; x++) {
			for(int i = 0; i < (int) pvector_size(&output); i++) {
				Point pnt = *(pvector_at(&output, i));

				if(pnt.x == x && pnt.y == y) {
					pvector_push(&sorted_output, pnt);
				}
			}
		}
	}


	return sorted_output;
}


//------------------------------------------------------------------------------

void render_pvector(pvector points) {
	for(int i = 0; i < (int) pvector_size(&points); i++) {
		Point pnt = *(pvector_at(&points, i));

		if(pnt.x != -1 && pnt.y != -1) DrawPixel(pnt.x, pnt.y, pnt.c);
	}
}

//------------------------------------------------------------------------------

int main(void) {
	pvector map = generate_starting_countries();

	InitWindow(WIDTH, HEIGHT, "Fanter!");

	RenderTexture2D tx = LoadRenderTexture(MAPWIDTH, MAPHEIGHT);

	while(!WindowShouldClose()) {

		Vector2 real_mp = (Vector2) { GetMouseX(), GetMouseY() };
		Vector2 grid_mp = (Vector2) { floor(real_mp.x / CELLSIZE), floor(real_mp.y / CELLSIZE) };

		BeginTextureMode(tx);
			render_pvector(map);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			DrawTextureEx(tx.texture, (Vector2) { 0, 0 }, 0.0f, (float) CELLSIZE, WHITE);

			DrawFPS(10, 10);

			DrawText(TextFormat("[%d, %d] [%d, %d]", grid_mp.x, grid_mp.y, real_mp.x, real_mp.y), 10, 40, 20, RAYWHITE);
		EndDrawing();

	}

	UnloadRenderTexture(tx);

	CloseWindow();

	return 0;

}

