import { Component } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MatTableModule } from '@angular/material/table';
import { MatButtonModule } from '@angular/material/button';
import { MatIconModule } from '@angular/material/icon';
import { MatCardModule } from '@angular/material/card';
import { MatToolbarModule } from '@angular/material/toolbar';

@Component({
  selector: 'app-bookmarks',
  standalone: true,
  imports: [
    CommonModule,
    MatTableModule,
    MatButtonModule,
    MatIconModule,
    MatCardModule,
    MatToolbarModule
  ],
  template: `
    <mat-toolbar color="primary">
      <span>Bookmarks</span>
      <span class="spacer"></span>
      <button mat-icon-button (click)="logout()">
        <mat-icon>logout</mat-icon>
      </button>
    </mat-toolbar>
    <div class="container">
      <mat-card>
        <mat-card-content>
          <table mat-table [dataSource]="bookmarks">
            <!-- Title Column -->
            <ng-container matColumnDef="title">
              <th mat-header-cell *matHeaderCellDef> Title </th>
              <td mat-cell *matCellDef="let bookmark"> {{bookmark.title}} </td>
            </ng-container>

            <!-- URL Column -->
            <ng-container matColumnDef="url">
              <th mat-header-cell *matHeaderCellDef> URL </th>
              <td mat-cell *matCellDef="let bookmark"> {{bookmark.url}} </td>
            </ng-container>

            <!-- Actions Column -->
            <ng-container matColumnDef="actions">
              <th mat-header-cell *matHeaderCellDef> Actions </th>
              <td mat-cell *matCellDef="let bookmark">
                <button mat-icon-button color="primary" (click)="editBookmark(bookmark)">
                  <mat-icon>edit</mat-icon>
                </button>
                <button mat-icon-button color="warn" (click)="deleteBookmark(bookmark)">
                  <mat-icon>delete</mat-icon>
                </button>
              </td>
            </ng-container>

            <tr mat-header-row *matHeaderRowDef="displayedColumns"></tr>
            <tr mat-row *matRowDef="let row; columns: displayedColumns;"></tr>
          </table>
        </mat-card-content>
      </mat-card>
    </div>
  `,
  styles: [`
    .container {
      padding: 20px;
    }
    .spacer {
      flex: 1 1 auto;
    }
    table {
      width: 100%;
    }
  `]
})
export class BookmarksComponent {
  displayedColumns: string[] = ['title', 'url', 'actions'];
  bookmarks: any[] = []; // TODO: Replace with proper interface

  constructor() {}

  logout() {
    // TODO: Implement logout
  }

  editBookmark(bookmark: any) {
    // TODO: Implement edit
  }

  deleteBookmark(bookmark: any) {
    // TODO: Implement delete
  }
} 