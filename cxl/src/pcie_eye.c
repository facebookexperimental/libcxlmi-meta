// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

/* std includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* libcxlmi includes */
#include <ccan/short_types/short_types.h>

/* vendor includes */
#include <vendor_types.h>

/* Helper functions for spd read */
void display_rx_setttings(struct rx_settings *settings) {
  if (!settings) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }
  printf("rx_settings: \n");
  printf("dlv0123 = [%d, %d, %d, %d],", settings->dlev00_signed,
         settings->dlev01_signed, settings->dlev10_signed,
         settings->dlev11_signed);
  printf("*vga, aeq = [%d, %d],", settings->vga, settings->aeq);
  printf("h2-9 = [%d, %d, %d, %d, %d, %d, %d, %d],", settings->h2, settings->h3,
         settings->h4, settings->h5, settings->h6, settings->h7, settings->h8,
         settings->h9);
  printf("appmd, rxrt, shd, wm = [%d, %d, 'g%d', %d],", settings->appmd,
         settings->rxrt, settings->shd, settings->wm);
  printf("h1ne/0, pe/o = [%d %d %d %d],", settings->h1ne, settings->h1no,
         settings->h1pe, settings->h1po);
  printf("iskw, qskw = [%d %d]", settings->iskew_signed,
         settings->qskew_signed);
  printf("\n");
}

void display_merged_eye_results(struct eyescope_results *eyescope_results) {

  if (!eyescope_results) {
    printf("Null pointer, cannot display structure\r\n");
    return;
  }

  printf("Merged Top (mV): %f\n", eyescope_results->merged_vertical_eye_top);
  printf("Merged Bottom (mV): %f\n",
         eyescope_results->merged_vertical_eye_bottom);
  printf("Merged Right Eye (UI): %f\n",
         eyescope_results->merged_horizontal_eye_right);
  printf("Merged Left Eye (UI): %f\n",
         eyescope_results->merged_horizontal_eye_left);
}
