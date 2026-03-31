using BookmarkManager.Data;
using BookmarkManager.DTOs;
using BookmarkManager.Models;
using Microsoft.EntityFrameworkCore;

namespace BookmarkManager.Services
{
    public class BookmarkService : IBookmarkService
    {
        private readonly ApplicationDbContext _context;
        private readonly IUserService _userService;

        public BookmarkService(ApplicationDbContext context, IUserService userService)
        {
            _context = context;
            _userService = userService;
        }

        public async Task<List<Bookmark>> GetAllBookmarksAsync(string username)
        {
            var user = await _userService.GetByUsernameAsync(username);
            return await _context.Bookmarks
                .Where(b => b.UserId == user.Id)
                .ToListAsync();
        }

        public async Task<List<Bookmark>> SearchBookmarksAsync(string username, string query)
        {
            var user = await _userService.GetByUsernameAsync(username);
            var lowercaseQuery = query.ToLower();
            
            return await _context.Bookmarks
                .Where(b => b.UserId == user.Id && 
                           (b.Title.ToLower().Contains(lowercaseQuery) || 
                            b.Url.ToLower().Contains(lowercaseQuery) || 
                            (b.Description != null && b.Description.ToLower().Contains(lowercaseQuery))))
                .ToListAsync();
        }

        public async Task<Bookmark> CreateBookmarkAsync(string username, BookmarkRequest bookmarkRequest)
        {
            var user = await _userService.GetByUsernameAsync(username);
            
            var bookmark = new Bookmark
            {
                Title = bookmarkRequest.Title,
                Url = bookmarkRequest.Url,
                Description = bookmarkRequest.Description,
                UserId = user.Id
            };

            _context.Bookmarks.Add(bookmark);
            await _context.SaveChangesAsync();
            
            return bookmark;
        }

        public async Task<Bookmark> UpdateBookmarkAsync(string username, int id, BookmarkRequest bookmarkRequest)
        {
            var user = await _userService.GetByUsernameAsync(username);
            var bookmark = await _context.Bookmarks.FindAsync(id);
            
            if (bookmark == null)
                throw new ApplicationException($"Bookmark not found with id: {id}");
                
            if (bookmark.UserId != user.Id)
                throw new ApplicationException("You don't have permission to update this bookmark");
            
            bookmark.Title = bookmarkRequest.Title;
            bookmark.Url = bookmarkRequest.Url;
            bookmark.Description = bookmarkRequest.Description;
            
            await _context.SaveChangesAsync();
            return bookmark;
        }

        public async Task DeleteBookmarkAsync(string username, int id)
        {
            var user = await _userService.GetByUsernameAsync(username);
            var bookmark = await _context.Bookmarks.FindAsync(id);
            
            if (bookmark == null)
                throw new ApplicationException($"Bookmark not found with id: {id}");
                
            if (bookmark.UserId != user.Id)
                throw new ApplicationException("You don't have permission to delete this bookmark");
                
            _context.Bookmarks.Remove(bookmark);
            await _context.SaveChangesAsync();
        }
    }
}
