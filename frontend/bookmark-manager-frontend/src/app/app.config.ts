/**
 * Main application configuration file
 * This file sets up the application's providers and imports necessary Angular modules
 */
import { ApplicationConfig, provideZoneChangeDetection } from '@angular/core';
import { provideRouter } from '@angular/router';
import { provideHttpClient } from '@angular/common/http';
import { provideAnimations } from '@angular/platform-browser/animations';
import { importProvidersFrom } from '@angular/core';
import { MatToolbarModule } from '@angular/material/toolbar';
import { MatButtonModule } from '@angular/material/button';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatTableModule } from '@angular/material/table';
import { MatIconModule } from '@angular/material/icon';
import { MatSnackBarModule } from '@angular/material/snack-bar';

import { routes } from './app.routes';

/**
 * Application configuration object
 * Provides routing, HTTP client, animations, and Angular Material modules
 */
export const appConfig: ApplicationConfig = {
  providers: [
    // Enable zone.js change detection with event coalescing for better performance
    provideZoneChangeDetection({ eventCoalescing: true }),
    // Set up the application's routing
    provideRouter(routes),
    // Enable HTTP client for API communication
    provideHttpClient(),
    // Enable animations for Angular Material components
    provideAnimations(),
    // Import Angular Material modules
    importProvidersFrom(
      MatToolbarModule,
      MatButtonModule,
      MatCardModule,
      MatFormFieldModule,
      MatInputModule,
      MatTableModule,
      MatIconModule,
      MatSnackBarModule
    )
  ]
};
