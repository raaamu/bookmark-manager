package com.example.bookmarkmanager.repository;

import com.example.bookmarkmanager.model.Bookmark;
import com.example.bookmarkmanager.model.User;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface BookmarkRepository extends JpaRepository<Bookmark, Long> {
    List<Bookmark> findByUser(User user);
    
    @Query("SELECT b FROM Bookmark b WHERE b.user = ?1 AND (LOWER(b.title) LIKE LOWER(CONCAT('%', ?2, '%')) OR LOWER(b.url) LIKE LOWER(CONCAT('%', ?2, '%')) OR LOWER(b.description) LIKE LOWER(CONCAT('%', ?2, '%')))")
    List<Bookmark> searchBookmarks(User user, String query);
}
