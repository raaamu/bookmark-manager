using BookmarkManager.DTOs;
using BookmarkManager.Models;

namespace BookmarkManager.Services
{
    public interface IBookmarkService
    {
        Task<List<Bookmark>> GetAllBookmarksAsync(string username);
        Task<List<Bookmark>> SearchBookmarksAsync(string username, string query);
        Task<Bookmark> CreateBookmarkAsync(string username, BookmarkRequest bookmarkRequest);
        Task<Bookmark> UpdateBookmarkAsync(string username, int id, BookmarkRequest bookmarkRequest);
        Task DeleteBookmarkAsync(string username, int id);
    }
}
