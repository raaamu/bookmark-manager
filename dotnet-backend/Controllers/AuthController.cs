using BookmarkManager.DTOs;
using BookmarkManager.Services;
using Microsoft.AspNetCore.Mvc;

namespace BookmarkManager.Controllers
{
    [ApiController]
    [Route("")]
    public class AuthController : ControllerBase
    {
        private readonly IUserService _userService;

        public AuthController(IUserService userService)
        {
            _userService = userService;
        }

        [HttpPost("register")]
        public async Task<IActionResult> Register([FromBody] RegisterRequest model)
        {
            try
            {
                var response = await _userService.RegisterAsync(model);
                return Ok(new { token = response.Token, message = "User registered successfully!" });
            }
            catch (ApplicationException ex)
            {
                return BadRequest(new { message = ex.Message });
            }
        }

        [HttpPost("login")]
        public async Task<IActionResult> Login([FromBody] LoginRequest model)
        {
            try
            {
                var response = await _userService.LoginAsync(model);
                return Ok(new { token = response.Token });
            }
            catch (ApplicationException ex)
            {
                return BadRequest(new { message = ex.Message });
            }
        }
    }
}
