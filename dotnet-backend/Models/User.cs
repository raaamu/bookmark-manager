using System.ComponentModel.DataAnnotations;
using System.Text.Json.Serialization;

namespace BookmarkManager.Models
{
    public class User
    {
        [Key]
        public int Id { get; set; }

        [Required]
        [StringLength(50)]
        public string Username { get; set; } = string.Empty;

        [Required]
        [StringLength(120)]
        [JsonIgnore]
        public string Password { get; set; } = string.Empty;

        [JsonIgnore]
        public List<Bookmark> Bookmarks { get; set; } = new List<Bookmark>();
    }
}
