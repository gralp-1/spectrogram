#include "../include/raylib.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N (1 << 11)
#define WIDTH 800
#define HEIGHT 600
#define ARRAY_LEN(xs) (sizeof(xs) / sizeof(xs[0]))
#define BACKGROUND                                                             \
  (Color) { 0x23, 0x29, 0x39, 0xFF }
typedef double complex cplx;

#include "debug.c"
#include "fft.c"
#include "quicksort.c"
float in[N];
cplx out_raw[N];
double out_clean[N];
double out_log[N];
double out_smooth[N];
double current_row[HEIGHT];
double history[WIDTH][HEIGHT];

int filter() {
  // apply the fast fourier transform
  fft(in, 1, out_raw, N);

  // find the maximum value to normalise
  float step = 1.005f;
  float lowf = 0.4f;
  int m = 0;
  float max_amp = 1.0f;
  for (float f = lowf; (size_t)f < N / 2; f = ceilf(f * step)) {
    float f1 = ceilf(f * step);
    float a = 0.0f;
    for (size_t q = (size_t)f; q < N / 2 && q < (size_t)f1; ++q) {
      float b = cabs(out_raw[q]);
      if (b > a)
        a = b;
    }
    if (max_amp < a)
      max_amp = a;
    out_log[m++] = a;
  }
  for (int i = 0; i < m; i++) {
    out_log[i] /= max_amp;
  }
  return m;
}

void DrawCentredText(const char *text, Font font, int size, Color color) {
  Vector2 offset = MeasureTextEx(font, text, size, 1);

  DrawText(text, GetScreenWidth() / 2 - offset.x / 2,
           GetScreenHeight() / 2 - offset.y / 2, size, color);
}

void in_push(float input, size_t len) {
  // add new audio samples to the input buffer, removing the oldest to create a
  // sliding window
  memmove(in, in + 1, (len - 1) * sizeof(in[0]));
  in[len - 1] = input;
}

void audio_callback(void *bufferData, unsigned int frames) {
  // every time a new audio buffer is ready, push it into the input buffer
  float(*fs)[2] = bufferData;
  for (size_t i = 0; i < frames; ++i) {
    in_push(fs[0][i], N);
  }
}

void history_push(double input[HEIGHT]) {
  // in a similar manner to in_push, add new rows (columns of pixels) to the
  // history buffer
  if (WIDTH > 1) {
    memmove(history, history + 1, (WIDTH - 1) * sizeof(history[0]));
  }
  memcpy(history[WIDTH - 1], input, HEIGHT * sizeof(double));
}
Color colour_scale(float amplitude) {

  // change the colour of the pixel depending on the amplitude
  // give more colour resolution to the lower amplitudes by using a log scale
  float log_amp = log10f(amplitude);
  float hue = log_amp * 360;
  Color color = ColorFromHSV(hue, 1, 1);

  return color;
}

void squish(double *inp, size_t inp_len, double *out, size_t desired_len) {
  // turn the N sized array into a smaller array, this will then be converted
  // into pixels so the array is just the right size for the screen height
  float step = (float)inp_len / desired_len;
  for (size_t i = 0; i < desired_len; i++) {
    out[i] = inp[(int)(i * step)];
  }
}

int main(void) {
  InitWindow(WIDTH, HEIGHT, "Spectrogram");
  InitAudioDevice();

  SetTargetFPS(60);
  char *file_path = NULL;
  Music music;
  bool playing = false;
  double pass_filter = 0.05;
  while (!WindowShouldClose()) {
    // Controls
    {
      if (IsKeyPressed(KEY_SPACE)) {
        playing = !playing;
        if (playing) {
          PauseMusicStream(music);
        } else {
          ResumeMusicStream(music);
        }
      }
      if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
        break;
      }
      if (IsKeyPressed(KEY_UP))
        pass_filter += 0.01;
      if (IsKeyPressed(KEY_DOWN))
        pass_filter -= 0.01;
    }
    BeginDrawing();
    ClearBackground(BACKGROUND);
    UpdateMusicStream(music);
    if (IsFileDropped()) {
      FilePathList files = LoadDroppedFiles();
      PauseMusicStream(music);
      UnloadMusicStream(music);
      if (files.count > 0)
        file_path = files.paths[0];

      UnloadDroppedFiles(files);
      music = LoadMusicStream(file_path);
      AttachAudioStreamProcessor(music.stream, audio_callback);
      PlayMusicStream(music);
    }
    if (IsAudioStreamPlaying(music.stream)) {
      // calculate how long to takes to apply filter
      int log_size =
          filter(); // apply the fast fourier transform and then clean
      // up the input audio buffer
      double *current_line = malloc(sizeof(double) * HEIGHT * 2);
      squish(out_log, log_size, current_line,
             HEIGHT *
                 2); // squish this line of frequencies down to a line of pixels
      history_push(current_line);
      // Draw the history from right to left
    }
    for (size_t x = 0; x < WIDTH; x++) {
      for (size_t y = 0; y < HEIGHT; y++) {
        if (history[x][HEIGHT - y] > pass_filter)
          DrawPixel(x, y, colour_scale(history[x][HEIGHT - y]));
      }
    }
    DrawFPS(10, 10);
    EndDrawing();
  }
  // deinitialisation
  UnloadMusicStream(music);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
