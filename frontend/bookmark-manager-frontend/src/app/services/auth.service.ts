/**
 * Authentication Service
 * Handles user authentication, registration, login, and logout operations
 * Manages authentication state and token storage
 */
import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { BehaviorSubject, Observable, throwError, timer, Subject } from 'rxjs';
import { tap, catchError } from 'rxjs/operators';
import { Router } from '@angular/router';

@Injectable({
  providedIn: 'root'
})
export class AuthService {
  /** Base API URL for authentication endpoints */
  private apiUrl = 'http://localhost:42000';
  
  /** Key used to store the authentication token in localStorage */
  private tokenKey = 'auth_token';
  
  /** Key used to store the token expiration timestamp in localStorage */
  private tokenExpiryKey = 'auth_token_expiry';
  
  /** Timer subscription for token expiration check */
  private tokenExpiryTimer: any;
  
  /** BehaviorSubject to track and broadcast authentication state changes */
  private isAuthenticatedSubject = new BehaviorSubject<boolean>(false);
  
  /** Subject to emit token expiry warnings with seconds remaining */
  private tokenExpiryWarning = new BehaviorSubject<number | null>(null);

  /**
   * Constructor
   * Initializes the authentication state based on token presence
   * 
   * @param http - HttpClient for API requests
   * @param router - Router for navigation
   */
  constructor(private http: HttpClient, private router: Router) {
    // Check if token is valid and not expired
    this.checkTokenValidity();
  }

  /**
   * Retrieves the authentication token from localStorage
   * 
   * @returns The authentication token or null if not found
   */
  getToken(): string | null {
    return localStorage.getItem(this.tokenKey);
  }

  /**
   * Retrieves the token expiration timestamp from localStorage
   * 
   * @returns The token expiration timestamp or null if not found
   */
  getTokenExpiry(): number | null {
    const expiry = localStorage.getItem(this.tokenExpiryKey);
    return expiry ? parseInt(expiry, 10) : null;
  }

  /**
   * Stores the authentication token in localStorage and updates authentication state
   * Also extracts and stores the expiration time from the JWT token
   * 
   * @param token - The authentication token to store
   */
  setToken(token: string): void {
    localStorage.setItem(this.tokenKey, token);
    
    // Extract expiration time from JWT token
    try {
      const tokenPayload = JSON.parse(atob(token.split('.')[1]));
      if (tokenPayload && tokenPayload.exp) {
        // Store expiration timestamp in milliseconds
        const expiryTime = tokenPayload.exp * 1000;
        localStorage.setItem(this.tokenExpiryKey, expiryTime.toString());
        
        // Set up expiration timer
        this.setupExpirationTimer(expiryTime);
      }
    } catch (error) {
      console.error('Error parsing JWT token:', error);
    }
    
    this.isAuthenticatedSubject.next(true);
  }

  /**
   * Removes the authentication token from localStorage and updates authentication state
   * Also clears the token expiration timer
   */
  removeToken(): void {
    localStorage.removeItem(this.tokenKey);
    localStorage.removeItem(this.tokenExpiryKey);
    this.isAuthenticatedSubject.next(false);
    
    // Clear any existing expiration timer
    if (this.tokenExpiryTimer) {
      clearTimeout(this.tokenExpiryTimer);
      this.tokenExpiryTimer = null;
    }
  }

  /**
   * Registers a new user
   * 
   * @param username - The username for registration
   * @param password - The password for registration
   * @returns Observable with registration response
   */
  register(username: string, password: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/register`, { username, password }).pipe(
      tap((response: any) => {
        console.log('Registration response:', response);
        if (response && response.token) {
          this.setToken(response.token);
        }
      }),
      catchError((error: HttpErrorResponse) => {
        console.error('Registration error:', error);
        return throwError(() => error);
      })
    );
  }

  /**
   * Logs in a user and stores the authentication token
   * 
   * @param username - The username for login
   * @param password - The password for login
   * @returns Observable with login response
   */
  login(username: string, password: string): Observable<any> {
    return this.http.post(`${this.apiUrl}/login`, { username, password }).pipe(
      tap((response: any) => {
        if (response && response.token) {
          this.setToken(response.token);
        }
      }),
      catchError((error: HttpErrorResponse) => {
        console.error('Login error:', error);
        return throwError(() => error);
      })
    );
  }

  /**
   * Logs out the current user and navigates to login page
   */
  logout(): void {
    this.removeToken();
    this.router.navigate(['/login']);
  }

  /**
   * Checks if the user is currently authenticated and the token is not expired
   * 
   * @returns Boolean indicating authentication status
   */
  isAuthenticated(): boolean {
    const token = this.getToken();
    if (!token) {
      return false;
    }
    
    const expiry = this.getTokenExpiry();
    if (!expiry) {
      return true; // If no expiry is found but token exists, assume it's valid
    }
    
    // Check if token is expired
    return Date.now() < expiry;
  }

  /**
   * Returns an observable of the authentication state
   * 
   * @returns Observable of authentication state
   */
  getAuthState(): Observable<boolean> {
    return this.isAuthenticatedSubject.asObservable();
  }
  
  /**
   * Returns an observable of token expiry warnings
   * 
   * @returns Observable that emits seconds remaining before expiry
   */
  getTokenExpiryWarning(): Observable<number | null> {
    return this.tokenExpiryWarning.asObservable();
  }

  /**
   * Route guard function to protect routes
   * Redirects to login if not authenticated
   * 
   * @returns Boolean indicating if route can be activated
   */
  canActivate(): boolean {
    const isAuth = this.isAuthenticated();
    if (!isAuth) {
      this.router.navigate(['/login']);
    }
    return isAuth;
  }
  
  /**
   * Sets up a timer to automatically log out when the token expires
   * 
   * @param expiryTime - Token expiration timestamp in milliseconds
   */
  private setupExpirationTimer(expiryTime: number): void {
    // Clear any existing timer
    if (this.tokenExpiryTimer) {
      clearTimeout(this.tokenExpiryTimer);
    }
    
    // Calculate time until expiration
    const timeUntilExpiry = expiryTime - Date.now();
    
    // Only set timer if expiration is in the future
    if (timeUntilExpiry > 0) {
      // Set timer to log out when token expires
      this.tokenExpiryTimer = setTimeout(() => {
        console.log('Token expired, logging out');
        this.logout();
      }, timeUntilExpiry);
      
      // Set a warning timer 1 minute before expiration
      if (timeUntilExpiry > 60000) {
        setTimeout(() => {
          // Emit an event for token expiry warning
          this.tokenExpiryWarning.next(60);
        }, timeUntilExpiry - 60000);
      }
    } else {
      // Token is already expired
      this.logout();
    }
  }
  
  /**
   * Checks if the stored token is valid and not expired
   * Updates authentication state accordingly
   */
  private checkTokenValidity(): void {
    const token = this.getToken();
    if (!token) {
      this.isAuthenticatedSubject.next(false);
    } else {
      const expiry = this.getTokenExpiry();
      if (expiry && Date.now() < expiry) {
        this.isAuthenticatedSubject.next(true);
        this.setupExpirationTimer(expiry);
      } else {
        this.removeToken();
      }
    }
  }
}
