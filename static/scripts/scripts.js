
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
 
async function performSearch() {
    const searchTerm = document.getElementById('penguin-search').value.trim();
    if (!searchTerm) return;

    try {
        const response = await fetch(`/api/search_penguin?id=${encodeURIComponent(searchTerm)}`);
        const data = await response.json();
        
        if (data.success) {
            window.location.href = data.redirect_url;
            
        } else {
            alert(data.message);
        }
    } catch (error) {
        console.error('Search failed:', error);
        alert('Search failed. Please try again.');
    }
}
