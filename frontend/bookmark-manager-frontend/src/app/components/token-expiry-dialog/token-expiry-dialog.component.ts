/**
 * Token Expiry Dialog Component
 * Displays a warning dialog when the JWT token is about to expire
 */
import { Component, Inject } from '@angular/core';
import { MAT_DIALOG_DATA, MatDialogRef, MatDialogModule } from '@angular/material/dialog';
import { MatButtonModule } from '@angular/material/button';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-token-expiry-dialog',
  standalone: true,
  imports: [
    CommonModule,
    MatDialogModule,
    MatButtonModule
  ],
  template: `
    <h2 mat-dialog-title>Session Expiring</h2>
    <mat-dialog-content>
      <p>Your session will expire in {{ data.timeLeft }} seconds. Would you like to stay logged in?</p>
    </mat-dialog-content>
    <mat-dialog-actions align="end">
      <button mat-button (click)="onLogout()">Logout</button>
      <button mat-button color="primary" (click)="onStayLoggedIn()">Stay Logged In</button>
    </mat-dialog-actions>
  `,
  styles: [`
    mat-dialog-content {
      min-width: 300px;
    }
  `]
})
export class TokenExpiryDialogComponent {
  /**
   * Constructor
   * 
   * @param dialogRef - Reference to the dialog
   * @param data - Data passed to the dialog
   */
  constructor(
    public dialogRef: MatDialogRef<TokenExpiryDialogComponent>,
    @Inject(MAT_DIALOG_DATA) public data: { timeLeft: number }
  ) {}

  /**
   * Handles the "Stay Logged In" button click
   * Closes the dialog with true result to indicate refresh token
   */
  onStayLoggedIn(): void {
    this.dialogRef.close(true);
  }

  /**
   * Handles the "Logout" button click
   * Closes the dialog with false result to indicate logout
   */
  onLogout(): void {
    this.dialogRef.close(false);
  }
}
