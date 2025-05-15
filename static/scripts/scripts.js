
document.addEventListener('DOMContentLoaded', function() {
    // Make sure video plays on mobile
    const video = document.querySelector('.video-background video');
    video.play().catch(e => {
        console.log("Autoplay prevented, showing fallback");
        video.controls = true;
    });

    // Smooth scrolling for navigation
    document.querySelectorAll('nav a').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            e.preventDefault();
            document.querySelector(this.getAttribute('href')).scrollIntoView({
                behavior: 'smooth'
            });
        });
    });
});

// Add to your existing script.js
const searchInput = document.querySelector('.search-input');
const searchButton = document.querySelector('.search-button');

// Focus animation
searchInput.addEventListener('focus', () => {
    searchButton.style.color = 'var(--primary-color)';
});

searchInput.addEventListener('blur', () => {
    searchButton.style.color = 'white';
});

// Search functionality
searchButton.addEventListener('click', () => {
    performSearch(searchInput.value);
});

searchInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        performSearch(searchInput.value);
    }
});

function performSearch(query) {
    console.log('Searching for:', query);
    // Add your actual search logic here
    alert(`Search functionality would look for: ${query}`);
}
function resizeVideo() {
    const video = document.getElementById('bg-video');
    const aspectRatio = video.videoWidth / video.videoHeight;
    const windowRatio = window.innerWidth / window.innerHeight;

    if (windowRatio > aspectRatio) {
        video.style.width = '100%';
        video.style.height = 'auto';
    } else {
        video.style.width = 'auto';
        video.style.height = '100%';
    }
}

// Initial resize
window.addEventListener('load', resizeVideo);

// Resize on window change
window.addEventListener('resize', resizeVideo);

// Also handle when video metadata loads
document.getElementById('bg-video').addEventListener('loadedmetadata', resizeVideo);

// Add to your existing script.js
// Add click tracking (in your existing script.js)
document.querySelectorAll('.article-card').forEach(card => {
    card.addEventListener('click', (e) => {
      if (!e.target.closest('.article-link')) {
        const title = card.querySelector('h3').textContent;
        console.log('Article clicked:', title);
        // window.location.href = `/articles/${title.toLowerCase().replace(/\s+/g, '-')}`;
      }
    });
  });
