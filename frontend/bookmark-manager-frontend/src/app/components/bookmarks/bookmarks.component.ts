/**
 * Bookmarks Component
 * Main component for managing bookmarks - displays, adds, and deletes bookmarks
 */
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormGroup, Validators, ReactiveFormsModule, FormControl } from '@angular/forms';
import { BookmarkService } from '../../services/bookmark.service';
import { AuthService } from '../../services/auth.service';
import { NotificationService } from '../../services/notification.service';
import { Router, RouterModule } from '@angular/router';
import { CommonModule } from '@angular/common';
import { MatToolbarModule } from '@angular/material/toolbar';
import { MatButtonModule } from '@angular/material/button';
import { MatCardModule } from '@angular/material/card';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatTableModule } from '@angular/material/table';
import { MatIconModule } from '@angular/material/icon';
import { MatSnackBarModule } from '@angular/material/snack-bar';
import { debounceTime, distinctUntilChanged } from 'rxjs/operators';

/**
 * Interface representing a bookmark entity
 */
interface Bookmark {
  /** Unique identifier for the bookmark */
  id: number;
  /** URL of the bookmark */
  url: string;
  /** Title/name of the bookmark */
  title: string;
  /** Optional description of the bookmark */
  description?: string;
}

@Component({
  selector: 'app-bookmarks',
  standalone: true,
  imports: [
    CommonModule,
    ReactiveFormsModule,
    RouterModule,
    MatToolbarModule,
    MatButtonModule,
    MatCardModule,
    MatFormFieldModule,
    MatInputModule,
    MatTableModule,
    MatIconModule,
    MatSnackBarModule
  ],
  templateUrl: './bookmarks.component.html',
  styleUrls: ['./bookmarks.component.scss']
})
export class BookmarksComponent implements OnInit {
  /** Array to store user's bookmarks */
  bookmarks: Bookmark[] = [];
  
  /** Form for adding new bookmarks */
  bookmarkForm: FormGroup;
  
  /** Search control for filtering bookmarks */
  searchControl = new FormControl('');
  
  /** Columns to display in the bookmarks table */
  displayedColumns: string[] = ['title', 'url', 'description', 'actions'];
  
  /** Flag to indicate if search is in progress */
  isSearching = false;

  /**
   * Constructor
   * Initializes the bookmark form with validation
   * 
   * @param bookmarkService - Service for bookmark CRUD operations
   * @param authService - Service for authentication operations
   * @param notificationService - Service for displaying notifications
   * @param router - Angular router for navigation
   * @param fb - FormBuilder for creating reactive forms
   */
  constructor(
    private bookmarkService: BookmarkService,
    private authService: AuthService,
    private notificationService: NotificationService,
    private router: Router,
    private fb: FormBuilder
  ) {
    // Initialize form with validation
    this.bookmarkForm = this.fb.group({
      // URL must be present and match http/https pattern
      url: ['', [Validators.required, Validators.pattern('https?://.+')]],
      // Title must be present
      title: ['', Validators.required],
      // Description is optional
      description: ['']
    });
  }

  /**
   * Lifecycle hook that runs when component initializes
   * Loads user's bookmarks from the API and sets up search
   */
  ngOnInit(): void {
    this.loadBookmarks();
    
    // Set up search with debounce to avoid too many API calls
    this.searchControl.valueChanges
      .pipe(
        debounceTime(300),
        distinctUntilChanged()
      )
      .subscribe(query => {
        this.isSearching = !!query;
        this.loadBookmarks(query || undefined);
      });
  }

  /**
   * Fetches bookmarks from the API and updates the component state
   * Displays error notification if loading fails
   * 
   * @param query - Optional search query to filter bookmarks
   */
  loadBookmarks(query?: string): void {
    this.bookmarkService.getBookmarks(query).subscribe({
      next: (bookmarks) => {
        this.bookmarks = bookmarks;
      },
      error: (error) => {
        console.error('Error loading bookmarks:', error);
        this.notificationService.showError('Failed to load bookmarks');
      }
    });
  }

  /**
   * Adds a new bookmark if the form is valid
   * Handles duplicate URL errors with appropriate notifications
   * Resets form on successful addition
   */
  addBookmark(): void {
    if (this.bookmarkForm.valid) {
      const { url, title, description } = this.bookmarkForm.value;
      this.bookmarkService.addBookmark(url, title, description).subscribe({
        next: () => {
          // Reload bookmarks to show the new addition
          this.loadBookmarks(this.searchControl.value || undefined);
          // Reset form for next entry
          this.bookmarkForm.reset();
          // Show success notification
          this.notificationService.showSuccess('Bookmark added successfully');
        },
        error: (error) => {
          console.error('Error adding bookmark:', error);
          // Special handling for duplicate URL error
          if (error.status === 409) {
            this.notificationService.showInfo('This URL has already been bookmarked');
          } else {
            this.notificationService.showError('Error adding bookmark: ' + (error.error?.message || 'Unknown error'));
          }
        }
      });
    }
  }

  /**
   * Deletes a bookmark by ID
   * Reloads bookmarks list after successful deletion
   * 
   * @param id - ID of the bookmark to delete
   */
  deleteBookmark(id: number): void {
    this.bookmarkService.deleteBookmark(id).subscribe({
      next: () => {
        // Reload bookmarks to reflect deletion
        this.loadBookmarks(this.searchControl.value || undefined);
        // Show success notification
        this.notificationService.showSuccess('Bookmark deleted successfully');
      },
      error: (error) => {
        console.error('Error deleting bookmark:', error);
        this.notificationService.showError('Failed to delete bookmark');
      }
    });
  }

  /**
   * Clears the search input and resets the bookmark list
   */
  clearSearch(): void {
    this.searchControl.setValue('');
    this.isSearching = false;
  }

  /**
   * Logs out the current user and navigates to login page
   * Shows logout notification
   */
  logout(): void {
    this.authService.logout();
    this.router.navigate(['/login']);
    this.notificationService.showInfo('You have been logged out');
  }
}
