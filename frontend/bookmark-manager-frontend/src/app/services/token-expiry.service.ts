/**
 * Token Expiry Service
 * Handles token expiration notifications and refresh
 */
import { Injectable } from '@angular/core';
import { MatDialog } from '@angular/material/dialog';
import { TokenExpiryDialogComponent } from '../components/token-expiry-dialog/token-expiry-dialog.component';
import { HttpClient } from '@angular/common/http';
import { Observable, of } from 'rxjs';
import { catchError, tap } from 'rxjs/operators';
import { AuthService } from './auth.service';

@Injectable({
  providedIn: 'root'
})
export class TokenExpiryService {
  /** Base API URL for authentication endpoints */
  private apiUrl = 'http://localhost:42000';
  
  /** Dialog reference to prevent multiple dialogs */
  private dialogOpen = false;

  /**
   * Constructor
   * 
   * @param dialog - MatDialog service for showing expiry dialog
   * @param http - HttpClient for API requests
   * @param authService - Authentication service
   */
  constructor(
    private dialog: MatDialog,
    private http: HttpClient,
    private authService: AuthService
  ) {}

  /**
   * Shows a token expiry warning dialog
   * 
   * @param secondsLeft - Seconds until token expires
   * @returns Observable that resolves when dialog is closed
   */
  showExpiryWarning(secondsLeft: number): Observable<boolean> {
    // Prevent multiple dialogs
    if (this.dialogOpen) {
      return of(false);
    }
    
    this.dialogOpen = true;
    
    const dialogRef = this.dialog.open(TokenExpiryDialogComponent, {
      width: '400px',
      disableClose: true,
      data: { timeLeft: secondsLeft }
    });

    return new Observable<boolean>(observer => {
      dialogRef.afterClosed().subscribe(result => {
        this.dialogOpen = false;
        
        if (result === true) {
          // User wants to stay logged in, try to refresh token
          this.refreshToken().subscribe({
            next: (success) => {
              observer.next(success);
              observer.complete();
            },
            error: () => {
              observer.next(false);
              observer.complete();
            }
          });
        } else {
          // User chose to logout
          observer.next(false);
          observer.complete();
        }
      });
    });
  }

  /**
   * Attempts to refresh the authentication token
   * 
   * @returns Observable indicating success or failure
   */
  refreshToken(): Observable<boolean> {
    const token = this.authService.getToken();
    
    if (!token) {
      return of(false);
    }
    
    return this.http.post<any>(`${this.apiUrl}/refresh-token`, {}, {
      headers: {
        'Authorization': `Bearer ${token}`
      }
    }).pipe(
      tap(response => {
        if (response && response.token) {
          this.authService.setToken(response.token);
          return true;
        }
        return false;
      }),
      catchError(() => {
        // If refresh fails, return false
        return of(false);
      })
    );
  }
}
