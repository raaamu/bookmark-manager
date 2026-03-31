/**
 * Login Component
 * Handles user authentication and login form
 */
import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Router, RouterModule } from '@angular/router';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatButtonModule } from '@angular/material/button';
import { AuthService } from '../services/auth.service';

@Component({
  selector: 'app-login',
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
    <div class="login-container">
      <mat-card>
        <mat-card-title>Login</mat-card-title>
        <mat-card-content>
          <form (ngSubmit)="onSubmit()">
            <mat-form-field>
              <input matInput placeholder="Username" [(ngModel)]="username" name="username" required>
            </mat-form-field>
            <mat-form-field>
              <input matInput type="password" placeholder="Password" [(ngModel)]="password" name="password" required>
            </mat-form-field>
            <button mat-raised-button color="primary" type="submit">Login</button>
            <p class="register-link">Don't have an account? <a [routerLink]="['/register']">Register</a></p>
          </form>
        </mat-card-content>
      </mat-card>
    </div>
  `,
  styles: [`
    .login-container {
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
    .register-link {
      text-align: center;
      margin-top: 1rem;
    }
  `]
})
export class LoginComponent {
  /** Username input field value */
  username: string = '';
  
  /** Password input field value */
  password: string = '';

  /**
   * Constructor
   * 
   * @param authService - Service for authentication operations
   * @param router - Angular router for navigation
   */
  constructor(private authService: AuthService, private router: Router) {}

  /**
   * Form submission handler
   * Attempts to log in with provided credentials
   * Navigates to bookmarks page on success
   */
  onSubmit() {
    this.authService.login(this.username, this.password).subscribe({
      next: () => {
        // Navigate to bookmarks page on successful login
        this.router.navigate(['/bookmarks']);
      },
      error: (error) => {
        console.error('Login failed:', error);
        // Note: Error handling could be improved with notifications
      }
    });
  }
}
