package com.example.bookmarkmanager.service;

import com.example.bookmarkmanager.model.Bookmark;
import com.example.bookmarkmanager.model.User;
import com.example.bookmarkmanager.repository.BookmarkRepository;
import com.example.bookmarkmanager.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class BookmarkService {
    @Autowired
    private BookmarkRepository bookmarkRepository;

    @Autowired
    private UserRepository userRepository;

    public List<Bookmark> getAllBookmarks(String username) {
        User user = userRepository.findByUsername(username)
                .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));
        return bookmarkRepository.findByUser(user);
    }

    public List<Bookmark> searchBookmarks(String username, String query) {
        User user = userRepository.findByUsername(username)
                .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));
        return bookmarkRepository.searchBookmarks(user, query);
    }

    public Bookmark createBookmark(String username, Bookmark bookmark) {
        User user = userRepository.findByUsername(username)
                .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));
        bookmark.setUser(user);
        return bookmarkRepository.save(bookmark);
    }

    public void deleteBookmark(String username, Long id) {
        User user = userRepository.findByUsername(username)
                .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));
        
        Bookmark bookmark = bookmarkRepository.findById(id)
                .orElseThrow(() -> new RuntimeException("Bookmark not found with id: " + id));
        
        if (!bookmark.getUser().getId().equals(user.getId())) {
            throw new RuntimeException("You don't have permission to delete this bookmark");
        }
        
        bookmarkRepository.deleteById(id);
    }
}
