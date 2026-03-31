package com.example.bookmarkmanager.controller;

import com.example.bookmarkmanager.model.Bookmark;
import com.example.bookmarkmanager.service.BookmarkService;
import jakarta.validation.Valid;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@CrossOrigin
@RestController
@RequestMapping("/bookmarks")
public class BookmarkController {
    @Autowired
    private BookmarkService bookmarkService;

    @GetMapping
    public ResponseEntity<List<Bookmark>> getBookmarks(Authentication authentication, 
                                                      @RequestParam(required = false) String q) {
        String username = authentication.getName();
        
        if (q != null && !q.isEmpty()) {
            return ResponseEntity.ok(bookmarkService.searchBookmarks(username, q));
        } else {
            return ResponseEntity.ok(bookmarkService.getAllBookmarks(username));
        }
    }

    @PostMapping
    public ResponseEntity<Bookmark> createBookmark(Authentication authentication, 
                                                  @Valid @RequestBody Bookmark bookmark) {
        String username = authentication.getName();
        Bookmark createdBookmark = bookmarkService.createBookmark(username, bookmark);
        return ResponseEntity.ok(createdBookmark);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<?> deleteBookmark(Authentication authentication, 
                                           @PathVariable Long id) {
        String username = authentication.getName();
        try {
            bookmarkService.deleteBookmark(username, id);
            return ResponseEntity.ok(Map.of("message", "Bookmark deleted successfully"));
        } catch (Exception e) {
            return ResponseEntity.badRequest().body(Map.of("message", e.getMessage()));
        }
    }
}
