/**
 * Register Component
 * Handles user registration and registration form
 */
import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Router, RouterModule } from '@angular/router';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { MatSnackBar } from '@angular/material/snack-bar';
import { AuthService } from '../services/auth.service';

@Component({
  selector: 'app-register',
  standalone: true,
  imports: [
    CommonModule,
    FormsModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatButtonModule,
    RouterModule
  ],
  template: `
    <div class="register-container">
      <mat-card>
        <mat-card-title>Register</mat-card-title>
        <mat-card-content>
          <form (ngSubmit)="onSubmit()">
            <mat-form-field>
              <input matInput placeholder="Username" [(ngModel)]="username" name="username" required minlength="3" maxlength="50">
              <mat-error *ngIf="username && username.length < 3">Username must be at least 3 characters</mat-error>
              <mat-error *ngIf="username && username.length > 50">Username must be at most 50 characters</mat-error>
            </mat-form-field>
            <mat-form-field>
              <input matInput type="password" placeholder="Password" [(ngModel)]="password" name="password" required minlength="6" maxlength="100">
              <mat-error *ngIf="password && password.length < 6">Password must be at least 6 characters</mat-error>
              <mat-error *ngIf="password && password.length > 100">Password must be at most 100 characters</mat-error>
            </mat-form-field>
            <button mat-raised-button color="primary" type="submit" [disabled]="!isFormValid()">Register</button>
            <p class="login-link">Already have an account? <a [routerLink]="['/login']">Login</a></p>
          </form>
        </mat-card-content>
      </mat-card>
    </div>
  `,
  styles: [`
    .register-container {
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }
    mat-card {
      width: 300px;
    }
    mat-form-field {
      width: 100%;
      margin-bottom: 1rem;
    }
    .login-link {
      text-align: center;
      margin-top: 1rem;
    }
  `]
})
export class RegisterComponent {
  /** Username input field value */
  username: string = '';
  
  /** Password input field value */
  password: string = '';

  /**
   * Constructor
   * 
   * @param authService - Service for authentication operations
   * @param router - Angular router for navigation
   * @param snackBar - Material snackbar for notifications
   */
  constructor(
    private authService: AuthService, 
    private router: Router,
    private snackBar: MatSnackBar
  ) {}

  /**
   * Checks if the form is valid
   * @returns boolean indicating if the form is valid
   */
  isFormValid(): boolean {
    return Boolean(this.username && this.password && 
           this.username.length >= 3 && this.username.length <= 50 &&
           this.password.length >= 6 && this.password.length <= 100);
  }

  /**
   * Form submission handler
   * Validates inputs and attempts to register a new user
   * Shows appropriate notifications for success/failure
   */
  onSubmit() {
    // Validate form inputs
    if (!this.isFormValid()) {
      this.snackBar.open('Please ensure username is 3-50 characters and password is 6-100 characters', 'Close', { duration: 3000 });
      return;
    }

    // Call registration service
    this.authService.register(this.username, this.password).subscribe({
      next: (response) => {
        console.log('Registration successful:', response);
        // Show success notification
        this.snackBar.open('Registration successful! Please login.', 'Close', { duration: 3000 });
        // Navigate to login page
        this.router.navigate(['/login']);
      },
      error: (error) => {
        console.error('Registration failed:', error);
        // Extract error message or use default
        const errorMessage = error.error?.message || 'Registration failed. Please try again.';
        // Show error notification
        this.snackBar.open(errorMessage, 'Close', { duration: 3000 });
      }
    });
  }
}
