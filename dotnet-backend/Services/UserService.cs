using BookmarkManager.Data;
using BookmarkManager.DTOs;
using BookmarkManager.Models;
using Microsoft.EntityFrameworkCore;

namespace BookmarkManager.Services
{
    public class UserService : IUserService
    {
        private readonly ApplicationDbContext _context;
        private readonly IJwtService _jwtService;

        public UserService(ApplicationDbContext context, IJwtService jwtService)
        {
            _context = context;
            _jwtService = jwtService;
        }

        public async Task<AuthResponse> RegisterAsync(RegisterRequest model)
        {
            if (await UsernameExistsAsync(model.Username))
                throw new ApplicationException("Username is already taken");

            var user = new User
            {
                Username = model.Username,
                Password = BCrypt.Net.BCrypt.HashPassword(model.Password)
            };

            _context.Users.Add(user);
            await _context.SaveChangesAsync();

            return new AuthResponse
            {
                Token = _jwtService.GenerateToken(user.Username)
            };
        }

        public async Task<AuthResponse> LoginAsync(LoginRequest model)
        {
            var user = await _context.Users
                .SingleOrDefaultAsync(u => u.Username == model.Username);

            if (user == null || !BCrypt.Net.BCrypt.Verify(model.Password, user.Password))
                throw new ApplicationException("Username or password is incorrect");

            return new AuthResponse
            {
                Token = _jwtService.GenerateToken(user.Username)
            };
        }

        public async Task<User> GetByUsernameAsync(string username)
        {
            var user = await _context.Users
                .SingleOrDefaultAsync(u => u.Username == username);

            if (user == null)
                throw new ApplicationException("User not found");

            return user;
        }

        public async Task<bool> UsernameExistsAsync(string username)
        {
            return await _context.Users.AnyAsync(u => u.Username == username);
        }
    }
}
