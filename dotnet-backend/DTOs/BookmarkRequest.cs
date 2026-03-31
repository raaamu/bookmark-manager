using System.ComponentModel.DataAnnotations;

namespace BookmarkManager.DTOs
{
    public class BookmarkRequest
    {
        [Required]
        [StringLength(100)]
        public string Title { get; set; } = string.Empty;

        [Required]
        [StringLength(500)]
        public string Url { get; set; } = string.Empty;

        [StringLength(1000)]
        public string? Description { get; set; }
    }
}
