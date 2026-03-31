using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Text.Json.Serialization;

namespace BookmarkManager.Models
{
    public class Bookmark
    {
        [Key]
        public int Id { get; set; }

        [Required]
        [StringLength(100)]
        public string Title { get; set; } = string.Empty;

        [Required]
        [StringLength(500)]
        public string Url { get; set; } = string.Empty;

        [StringLength(1000)]
        public string? Description { get; set; }

        [Required]
        public int UserId { get; set; }

        [ForeignKey("UserId")]
        [JsonIgnore]
        public User? User { get; set; }
    }
}
