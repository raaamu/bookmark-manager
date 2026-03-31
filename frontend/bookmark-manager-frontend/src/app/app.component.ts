import { Component, OnInit, OnDestroy } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { CommonModule } from '@angular/common';
import { AuthService } from './services/auth.service';
import { TokenExpiryService } from './services/token-expiry.service';
import { Subscription } from 'rxjs';
import { MatSnackBar } from '@angular/material/snack-bar';
import { MatSnackBarModule } from '@angular/material/snack-bar';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [
    CommonModule,
    RouterOutlet,
    MatSnackBarModule
  ],
  template: `
    <div class="app-container">
      <router-outlet></router-outlet>
    </div>
  `,
  styles: [`
    .app-container {
      height: 100vh;
      width: 100vw;
    }
  `]
})
export class AppComponent implements OnInit, OnDestroy {
  title = 'bookmark-manager-frontend';
  
  /** Subscription to token expiry warnings */
  private tokenExpirySubscription: Subscription | null = null;
  
  /**
   * Constructor
   * 
   * @param authService - Authentication service
   * @param tokenExpiryService - Token expiry service
   * @param snackBar - Material snackbar for notifications
   */
  constructor(
    private authService: AuthService,
    private tokenExpiryService: TokenExpiryService,
    private snackBar: MatSnackBar
  ) {}
  
  /**
   * Lifecycle hook that runs when component initializes
   * Sets up subscription to token expiry warnings
   */
  ngOnInit(): void {
    this.tokenExpirySubscription = this.authService.getTokenExpiryWarning().subscribe(
      secondsLeft => {
        if (secondsLeft) {
          this.handleTokenExpiry(secondsLeft);
        }
      }
    );
  }
  
  /**
   * Lifecycle hook that runs when component is destroyed
   * Cleans up subscriptions
   */
  ngOnDestroy(): void {
    if (this.tokenExpirySubscription) {
      this.tokenExpirySubscription.unsubscribe();
      this.tokenExpirySubscription = null;
    }
  }
  
  /**
   * Handles token expiry warning
   * Shows dialog and handles user response
   * 
   * @param secondsLeft - Seconds until token expires
   */
  private handleTokenExpiry(secondsLeft: number): void {
    // First show a snackbar notification
    const snackBarRef = this.snackBar.open(
      'Your session is about to expire',
      'Extend Session',
      { duration: 10000 }
    );
    
    snackBarRef.onAction().subscribe(() => {
      // When user clicks "Extend Session", show the dialog
      this.tokenExpiryService.showExpiryWarning(secondsLeft).subscribe(
        refreshed => {
          if (!refreshed) {
            // If token wasn't refreshed, log the user out
            this.authService.logout();
          }
        }
      );
    });
  }
}
