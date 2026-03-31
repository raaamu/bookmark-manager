/**
 * Application routing configuration
 * Defines the routes and their corresponding components
 */
import { Routes } from '@angular/router';
import { LoginComponent } from './login/login.component';
import { RegisterComponent } from './register/register.component';
import { BookmarksComponent } from './components/bookmarks/bookmarks.component';
import { AuthService } from './services/auth.service';
import { inject } from '@angular/core';

/**
 * Application routes
 * - Default route redirects to login
 * - Login and register routes are publicly accessible
 * - Bookmarks route is protected by authentication guard
 */
export const routes: Routes = [
  // Redirect to login page by default
  { path: '', redirectTo: '/login', pathMatch: 'full' },
  
  // Login page
  { path: 'login', component: LoginComponent },
  
  // Registration page
  { path: 'register', component: RegisterComponent },
  
  // Bookmarks page (protected by authentication)
  { 
    path: 'bookmarks', 
    component: BookmarksComponent,
    // Functional route guard that checks if user is authenticated
    canActivate: [() => inject(AuthService).isAuthenticated()]
  }
];
