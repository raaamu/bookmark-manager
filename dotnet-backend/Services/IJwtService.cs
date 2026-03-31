using System.Security.Claims;

namespace BookmarkManager.Services
{
    public interface IJwtService
    {
        string GenerateToken(string username);
        string? ValidateToken(string token);
        ClaimsPrincipal GetPrincipalFromToken(string token);
    }
}
