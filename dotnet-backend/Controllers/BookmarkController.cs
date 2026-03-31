using BookmarkManager.DTOs;
using BookmarkManager.Models;
using BookmarkManager.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using System.Security.Claims;

namespace BookmarkManager.Controllers
{
    [ApiController]
    [Authorize]
    [Route("bookmarks")]
    public class BookmarkController : ControllerBase
    {
        private readonly IBookmarkService _bookmarkService;

        public BookmarkController(IBookmarkService bookmarkService)
        {
            _bookmarkService = bookmarkService;
        }

        [HttpGet]
        public async Task<ActionResult<IEnumerable<Bookmark>>> GetBookmarks([FromQuery] string? q)
        {
            var username = User.FindFirst(ClaimTypes.Name)?.Value;
            if (username == null)
                return Unauthorized();

            if (!string.IsNullOrEmpty(q))
            {
                var searchResults = await _bookmarkService.SearchBookmarksAsync(username, q);
                return Ok(searchResults);
            }
            else
            {
                var bookmarks = await _bookmarkService.GetAllBookmarksAsync(username);
                return Ok(bookmarks);
            }
        }

        [HttpPost]
        public async Task<ActionResult<Bookmark>> CreateBookmark([FromBody] BookmarkRequest bookmarkRequest)
        {
            var username = User.FindFirst(ClaimTypes.Name)?.Value;
            if (username == null)
                return Unauthorized();

            try
            {
                var bookmark = await _bookmarkService.CreateBookmarkAsync(username, bookmarkRequest);
                return Ok(bookmark);
            }
            catch (ApplicationException ex)
            {
                return BadRequest(new { message = ex.Message });
            }
        }

        [HttpPut("{id:int}")]
        public async Task<ActionResult<Bookmark>> UpdateBookmark(int id, [FromBody] BookmarkRequest bookmarkRequest)
        {
            var username = User.FindFirst(ClaimTypes.Name)?.Value;
            if (username == null)
                return Unauthorized();

            try
            {
                var bookmark = await _bookmarkService.UpdateBookmarkAsync(username, id, bookmarkRequest);
                return Ok(bookmark);
            }
            catch (ApplicationException ex)
            {
                return BadRequest(new { message = ex.Message });
            }
        }

        [HttpDelete("{id}")]
        public async Task<IActionResult> DeleteBookmark(int id)
        {
            var username = User.FindFirst(ClaimTypes.Name)?.Value;
            if (username == null)
                return Unauthorized();

            try
            {
                await _bookmarkService.DeleteBookmarkAsync(username, id);
                return Ok(new { message = "Bookmark deleted successfully" });
            }
            catch (ApplicationException ex)
            {
                return BadRequest(new { message = ex.Message });
            }
        }
    }
}
