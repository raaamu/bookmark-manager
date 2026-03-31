using BookmarkManager.DTOs;
using BookmarkManager.Models;

namespace BookmarkManager.Services
{
    public interface IUserService
    {
        Task<AuthResponse> RegisterAsync(RegisterRequest model);
        Task<AuthResponse> LoginAsync(LoginRequest model);
        Task<User> GetByUsernameAsync(string username);
        Task<bool> UsernameExistsAsync(string username);
    }
}
