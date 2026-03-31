package com.example.bookmarkmanager.service;

import com.example.bookmarkmanager.model.Bookmark;
import com.example.bookmarkmanager.model.User;
import com.example.bookmarkmanager.repository.BookmarkRepository;
import com.example.bookmarkmanager.repository.UserRepository;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import org.junit.Test;
import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import static org.junit.Assert.*;
import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.mockito.Mockito.*;
import static org.mockito.Mockito.when;

import static org.junit.Assert.assertEquals;

@SpringBootTest
@RunWith(MockitoJUnitRunner.class)
public class BookmarkServiceTest {

    @Mock
    private BookmarkRepository bookmarkRepository;

    @InjectMocks
    private BookmarkService bookmarkService;

    @Mock
    private UserRepository userRepository;

    /**
     * Test case for creating a bookmark with a non-existent username.
     * This test verifies that the createBookmark method throws a UsernameNotFoundException
     * when attempting to create a bookmark for a user that doesn't exist in the system.
     */
    @Test
    public void testCreateBookmark_NonExistentUser() {
        String nonExistentUsername = "nonexistentuser";
        Bookmark bookmark = new Bookmark();

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.createBookmark(nonExistentUsername, bookmark);
        });
    }

    /**
     * Tests the searchBookmarks method with a non-existent username.
     * This test verifies that the method throws a UsernameNotFoundException
     * when the provided username does not exist in the user repository.
     */
    @Test
    public void testSearchBookmarks_NonExistentUsername() {
        String nonExistentUsername = "nonexistentuser";
        String query = "test";

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.searchBookmarks(nonExistentUsername, query);
        });
    }

    /**
     * Test case for createBookmark method when user exists and bookmark is successfully created.
     * This test verifies that:
     * 1. The user is found by username
     * 2. The bookmark is associated with the user
     * 3. The bookmark is saved in the repository
     * 4. The saved bookmark is returned
     */
    @Test
    public void test_createBookmark_userExistsAndBookmarkCreated() {
        // Arrange
        String username = "testUser";
        User user = new User();
        user.setUsername(username);

        Bookmark bookmark = new Bookmark();
        bookmark.setUrl("https://example.com");

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.save(any(Bookmark.class))).thenReturn(bookmark);

        // Act
        Bookmark result = bookmarkService.createBookmark(username, bookmark);

        // Assert
        assertNotNull(result);
        assertEquals(user, result.getUser());
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).save(bookmark);
    }

    /**
     * Test case for deleteBookmark method when the bookmark is not found.
     * This test verifies that a RuntimeException is thrown when
     * trying to delete a non-existent bookmark.
     */
    @Test
    public void test_deleteBookmark_bookmarkNotFound() {
        String username = "existingUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.empty());

        assertThrows(RuntimeException.class, () -> {
            bookmarkService.deleteBookmark(username, bookmarkId);
        });

        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).findById(bookmarkId);
    }

    /**
     * Test case for deleteBookmark method when the user doesn't have permission to delete the bookmark.
     * This test verifies that a RuntimeException is thrown when a user tries to delete
     * a bookmark that belongs to another user.
     */
    @Test
    public void test_deleteBookmark_userDoesNotHavePermission() {
        String username = "existingUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        User otherUser = new User();
        otherUser.setId(2L);
        Bookmark bookmark = new Bookmark();
        bookmark.setUser(otherUser);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        assertThrows(RuntimeException.class, () -> {
            bookmarkService.deleteBookmark(username, bookmarkId);
        });

        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).findById(bookmarkId);
        verify(bookmarkRepository, never()).deleteById(anyLong());
    }

    /**
     * Test case for deleteBookmark method when the user is not found.
     * This test verifies that a UsernameNotFoundException is thrown when
     * trying to delete a bookmark for a non-existent user.
     */
    @Test
    public void test_deleteBookmark_userNotFound() {
        String username = "nonexistentUser";
        Long bookmarkId = 1L;

        when(userRepository.findByUsername(username)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.deleteBookmark(username, bookmarkId);
        });

        verify(userRepository).findByUsername(username);
        verifyNoInteractions(bookmarkRepository);
    }

    /**
     * Test case for deleteBookmark method when the user doesn't have permission to delete the bookmark.
     * This test verifies that a RuntimeException is thrown when the bookmark's user ID doesn't match
     * the authenticated user's ID.
     */
    @Test(expected = RuntimeException.class)
    public void test_deleteBookmark_whenUserDoesNotHavePermission() {
        // Arrange
        String username = "testUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        Bookmark bookmark = new Bookmark();
        User bookmarkUser = new User();
        bookmarkUser.setId(2L);
        bookmark.setUser(bookmarkUser);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        // Act
        bookmarkService.deleteBookmark(username, bookmarkId);

        // Assert
        // The expected RuntimeException should be thrown
    }

    /**
     * Test case for deleting a bookmark when the user has permission.
     * This test verifies that the deleteBookmark method successfully deletes a bookmark
     * when the user owns the bookmark and has the permission to delete it.
     */
    @Test
    public void test_deleteBookmark_whenUserHasPermission() {
        // Arrange
        String username = "testUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        user.setUsername(username);

        Bookmark bookmark = new Bookmark();
        bookmark.setId(bookmarkId);
        bookmark.setUser(user);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        // Act
        bookmarkService.deleteBookmark(username, bookmarkId);

        // Assert
        verify(bookmarkRepository).deleteById(bookmarkId);
    }

    /**
     * Tests the getAllBookmarks method with a non-existent username.
     * This test verifies that the method throws a UsernameNotFoundException
     * when the provided username does not exist in the userRepository.
     */
    @Test
    public void test_getAllBookmarks_nonExistentUser() {
        String nonExistentUsername = "nonexistent_user";
        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.getAllBookmarks(nonExistentUsername);
        });
    }

    /**
     * Test case for getAllBookmarks method when the user exists and has bookmarks.
     * It verifies that the method correctly retrieves all bookmarks for a given user.
     */
    @Test
    public void test_getAllBookmarks_userExistsWithBookmarks() {
        // Arrange
        String username = "testUser";
        User user = new User();
        user.setUsername(username);

        Bookmark bookmark1 = new Bookmark();
        Bookmark bookmark2 = new Bookmark();
        List<Bookmark> expectedBookmarks = Arrays.asList(bookmark1, bookmark2);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findByUser(user)).thenReturn(expectedBookmarks);

        // Act
        List<Bookmark> actualBookmarks = bookmarkService.getAllBookmarks(username);

        // Assert
        assertEquals(expectedBookmarks, actualBookmarks);
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).findByUser(user);
    }

    /**
     * Test case for searchBookmarks method when the user exists and bookmarks are found.
     * This test verifies that the method correctly retrieves and returns bookmarks
     * for a given user and search query.
     */
    @Test
    public void test_searchBookmarks_whenUserExistsAndBookmarksFound() {
        // Arrange
        String username = "testUser";
        String query = "test";
        User user = new User();
        user.setUsername(username);

        Bookmark bookmark1 = new Bookmark();
        bookmark1.setTitle("Test Bookmark 1");
        Bookmark bookmark2 = new Bookmark();
        bookmark2.setTitle("Test Bookmark 2");
        List<Bookmark> expectedBookmarks = Arrays.asList(bookmark1, bookmark2);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.searchBookmarks(user, query)).thenReturn(expectedBookmarks);

        // Act
        List<Bookmark> result = bookmarkService.searchBookmarks(username, query);

        // Assert
        assertEquals(expectedBookmarks, result);
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).searchBookmarks(user, query);
    }


    /**
     * Test case for deleteBookmark method when the user doesn't have permission to delete the bookmark.
     * This test verifies that a RuntimeException is thrown when the bookmark's user ID
     * does not match the ID of the user attempting to delete the bookmark.
     */
    @Test
    public void testDeleteBookmark_UserDoesNotHavePermission() {
        String username = "testUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        Bookmark bookmark = new Bookmark();
        User bookmarkUser = new User();
        bookmarkUser.setId(2L);
        bookmark.setUser(bookmarkUser);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        assertThrows(RuntimeException.class, () -> {
            bookmarkService.deleteBookmark(username, bookmarkId);
        });

        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).findById(bookmarkId);
        verify(bookmarkRepository, never()).deleteById(bookmarkId);
    }

    /**
     * Test case for createBookmark method when the user exists and bookmark is created successfully.
     * This test verifies that the method correctly creates a new bookmark for a given user
     * and returns the saved bookmark.
     */
    @Test
    public void test_createBookmark_userExistsAndBookmarkCreated_2() {
        // Arrange
        String username = "testUser";
        User user = new User();
        user.setUsername(username);
        Bookmark bookmark = new Bookmark();
        bookmark.setTitle("Test Bookmark");

        when(userRepository.findByUsername(username)).thenReturn(java.util.Optional.of(user));
        when(bookmarkRepository.save(bookmark)).thenReturn(bookmark);

        // Act
        Bookmark result = bookmarkService.createBookmark(username, bookmark);

        // Assert
        assertNotNull(result);
        assertEquals(bookmark, result);
        assertEquals(user, bookmark.getUser());
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).save(bookmark);
    }

    /**
     * Tests the createBookmark method when the user is not found.
     * This test verifies that a UsernameNotFoundException is thrown when
     * attempting to create a bookmark for a non-existent user.
     */
    @Test
    public void test_createBookmark_userNotFound() {
        String nonExistentUsername = "nonexistentuser";
        Bookmark bookmark = new Bookmark();

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.createBookmark(nonExistentUsername, bookmark);
        });

        verify(userRepository).findByUsername(nonExistentUsername);
        verifyNoInteractions(bookmarkRepository);
    }

    /**
     * Test case for deleting a bookmark when the bookmark is not found.
     * This test verifies that the deleteBookmark method throws a RuntimeException
     * when attempting to delete a bookmark that doesn't exist in the system.
     */
    @Test
    public void test_deleteBookmark_bookmarkNotFound_2() {
        String username = "testuser";
        Long nonExistentBookmarkId = 999L;
        User user = new User();
        user.setId(1L);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(nonExistentBookmarkId)).thenReturn(Optional.empty());

        assertThrows(RuntimeException.class, () -> {
            bookmarkService.deleteBookmark(username, nonExistentBookmarkId);
        });
    }

    /**
     * Test case for deleting a bookmark when the user has permission.
     * This test verifies that:
     * 1. The user is found by username
     * 2. The bookmark is found by id
     * 3. The user has permission to delete the bookmark
     * 4. The bookmark is successfully deleted
     */
    @Test
    public void test_deleteBookmark_userHasPermission() {
        // Arrange
        String username = "testUser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        Bookmark bookmark = new Bookmark();
        bookmark.setId(bookmarkId);
        bookmark.setUser(user);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        // Act
        bookmarkService.deleteBookmark(username, bookmarkId);

        // Assert
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).findById(bookmarkId);
        verify(bookmarkRepository).deleteById(bookmarkId);
    }

    /**
     * Test case for deleting a bookmark when the user doesn't have permission.
     * This test verifies that the deleteBookmark method throws a RuntimeException
     * when attempting to delete a bookmark that belongs to a different user.
     */
    @Test
    public void test_deleteBookmark_userNoPermission() {
        String username = "testuser";
        Long bookmarkId = 1L;
        User user = new User();
        user.setId(1L);
        User otherUser = new User();
        otherUser.setId(2L);
        Bookmark bookmark = new Bookmark();
        bookmark.setUser(otherUser);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findById(bookmarkId)).thenReturn(Optional.of(bookmark));

        assertThrows(RuntimeException.class, () -> {
            bookmarkService.deleteBookmark(username, bookmarkId);
        });
    }

    /**
     * Test case for deleting a bookmark when the user is not found.
     * This test verifies that the deleteBookmark method throws a UsernameNotFoundException
     * when attempting to delete a bookmark for a user that doesn't exist in the system.
     */
    @Test
    public void test_deleteBookmark_userNotFound_2() {
        String nonExistentUsername = "nonexistentuser";
        Long bookmarkId = 1L;

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.deleteBookmark(nonExistentUsername, bookmarkId);
        });
    }

    /**
     * Test case for getAllBookmarks method when the user exists and has bookmarks.
     * This test verifies that the method correctly retrieves and returns all bookmarks
     * for a given user.
     */
    @Test
    public void test_getAllBookmarks_userExistsWithBookmarks_2() {
        // Arrange
        String username = "testUser";
        User user = new User();
        user.setUsername(username);

        Bookmark bookmark1 = new Bookmark();
        bookmark1.setTitle("Bookmark 1");
        Bookmark bookmark2 = new Bookmark();
        bookmark2.setTitle("Bookmark 2");
        List<Bookmark> expectedBookmarks = Arrays.asList(bookmark1, bookmark2);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.findByUser(user)).thenReturn(expectedBookmarks);

        // Act
        List<Bookmark> result = bookmarkService.getAllBookmarks(username);

        // Assert
        assertEquals(expectedBookmarks, result);
    }

    /**
     * Test case for getAllBookmarks method when the user is not found.
     * This test verifies that a UsernameNotFoundException is thrown when
     * trying to get all bookmarks for a non-existent user.
     */
    @Test
    public void test_getAllBookmarks_userNotFound() {
        String nonExistentUsername = "nonexistentuser";

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.getAllBookmarks(nonExistentUsername);
        });

        verify(userRepository).findByUsername(nonExistentUsername);
        verifyNoInteractions(bookmarkRepository);
    }

    /**
     * Test case for searching bookmarks with a non-existent username.
     * This test verifies that the searchBookmarks method throws a UsernameNotFoundException
     * when attempting to search bookmarks for a user that doesn't exist in the system.
     */
    @Test
    public void test_searchBookmarks_nonExistentUser() {
        String nonExistentUsername = "nonexistentuser";
        String query = "test";

        when(userRepository.findByUsername(nonExistentUsername)).thenReturn(Optional.empty());

        assertThrows(UsernameNotFoundException.class, () -> {
            bookmarkService.searchBookmarks(nonExistentUsername, query);
        });
    }

    /**
     * Test case for searchBookmarks method when the user exists and bookmarks are found.
     * This test verifies that:
     * 1. The user is found by username
     * 2. The bookmarks are searched using the provided query
     * 3. The searched bookmarks are returned
     */
    @Test
    public void test_searchBookmarks_userExistsAndBookmarksFound() {
        // Arrange
        String username = "testUser";
        String query = "test";
        User user = new User();
        user.setUsername(username);

        Bookmark bookmark1 = new Bookmark();
        Bookmark bookmark2 = new Bookmark();
        List<Bookmark> expectedBookmarks = Arrays.asList(bookmark1, bookmark2);

        when(userRepository.findByUsername(username)).thenReturn(Optional.of(user));
        when(bookmarkRepository.searchBookmarks(user, query)).thenReturn(expectedBookmarks);

        // Act
        List<Bookmark> actualBookmarks = bookmarkService.searchBookmarks(username, query);

        // Assert
        assertEquals(expectedBookmarks, actualBookmarks);
        verify(userRepository).findByUsername(username);
        verify(bookmarkRepository).searchBookmarks(user, query);
    }
}
