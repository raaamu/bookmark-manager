/**
 * Bookmark Service
 * Handles all bookmark-related API operations
 * Manages CRUD operations for bookmarks
 */
import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders, HttpParams } from '@angular/common/http';
import { Observable } from 'rxjs';
import { AuthService } from './auth.service';

@Injectable({
  providedIn: 'root'
})
export class BookmarkService {
  /** Base API URL for bookmark endpoints */
  private apiUrl = 'http://localhost:42000';

  /**
   * Constructor
   * 
   * @param http - HttpClient for API requests
   * @param authService - AuthService for authentication token
   */
  constructor(private http: HttpClient, private authService: AuthService) {}

  /**
   * Creates HTTP headers with authentication token
   * 
   * @returns HttpHeaders with Authorization and Content-Type headers
   */
  private getHeaders(): HttpHeaders {
    const token = this.authService.getToken();
    return new HttpHeaders({
      'Authorization': `Bearer ${token}`,
      'Content-Type': 'application/json'
    });
  }

  /**
   * Fetches all bookmarks for the authenticated user
   * 
   * @param query - Optional search query to filter bookmarks
   * @returns Observable with array of bookmark objects
   */
  getBookmarks(query?: string): Observable<any> {
    let params = new HttpParams();
    if (query) {
      params = params.set('q', query);
    }
    
    return this.http.get(`${this.apiUrl}/bookmarks`, { 
      headers: this.getHeaders(),
      params: params
    });
  }

  /**
   * Adds a new bookmark
   * 
   * @param url - URL of the bookmark
   * @param title - Title of the bookmark
   * @param description - Optional description of the bookmark
   * @returns Observable with API response
   */
  addBookmark(url: string, title: string, description?: string): Observable<any> {
    return this.http.post(
      `${this.apiUrl}/bookmarks`,
      { url, title, description },
      { headers: this.getHeaders() }
    );
  }

  /**
   * Deletes a bookmark by ID
   * 
   * @param id - ID of the bookmark to delete
   * @returns Observable with API response
   */
  deleteBookmark(id: number): Observable<any> {
    return this.http.delete(
      `${this.apiUrl}/bookmarks/${id}`,
      { headers: this.getHeaders() }
    );
  }
}
