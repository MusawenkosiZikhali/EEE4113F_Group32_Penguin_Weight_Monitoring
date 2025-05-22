function filterPenguins() {
    // Get filter values (similar to beaches.html)
    const searchText = document.getElementById('id-search').value.toLowerCase();
    const selectedLocation = document.getElementById('location-filter').value;
    const selectedWeight = document.getElementById('weight-filter').value;
    
    // Select all penguin cards (like beachCards)
    const penguinCards = document.querySelectorAll('.penguin-card');

    penguinCards.forEach(card => {
        // Get penguin data (similar to beach data extraction)
        const penguinId = card.querySelector('h3').textContent.toLowerCase();
        const location = card.querySelector('p:nth-of-type(1)').textContent.toLowerCase();
        const weightText = card.querySelector('p:nth-of-type(2)').textContent;
        const weight = parseFloat(weightText.match(/\d+\.?\d*/)[0]);
        const condition = card.querySelector('p:nth-of-type(3)').textContent.toLowerCase();

        // Create filter conditions (like matchesfilter/matchesSafety)
        const matchesSearch = searchText === '' || 
                            penguinId.includes(searchText) || 
                            location.includes(searchText);
        
        const matchesLocation = selectedLocation === 'all' || 
                              location.includes(selectedLocation.toLowerCase());
        
        // Weight filter logic (specific to penguins)
        let matchesWeight = true;
        if (selectedWeight !== 'all') {
            if (selectedWeight === 'overweight' && weight <= 4.5) matchesWeight = false;
            if (selectedWeight === 'underweight' && weight >= 3) matchesWeight = false;
            if (selectedWeight === 'normal' && (weight < 3 || weight > 4.5)) matchesWeight = false;
        }

        // Show/Hide based on conditions (same pattern as beaches.html)
        if (matchesSearch && matchesLocation && matchesWeight) {
            card.style.display = 'block'; // Show card
        } else {
            card.style.display = 'none'; // Hide card
        }
    });
}

// Initialize event listeners (similar pattern)
document.addEventListener('DOMContentLoaded', function() {
    // Text search
    document.getElementById('id-search').addEventListener('input', filterPenguins);
    
    // Dropdown filters
    document.getElementById('location-filter').addEventListener('change', filterPenguins);
    document.getElementById('weight-filter').addEventListener('change', filterPenguins);
    
    // Apply button
    document.getElementById('apply-filters').addEventListener('click', filterPenguins);
});

let currentPage = 1;
const cardsPerPage = 15;
let allPenguinCards = [];
let filteredPenguinCards = [];

document.addEventListener('DOMContentLoaded', function() {
    allPenguinCards = Array.from(document.querySelectorAll('.penguin-card'));
    filteredPenguinCards = [...allPenguinCards];
    updatePage();
});

function updatePage() {
    // Hide all cards first
    allPenguinCards.forEach(card => card.style.display = 'none');
    
    // Calculate start and end index
    const startIndex = (currentPage - 1) * cardsPerPage;
    const endIndex = startIndex + cardsPerPage;
    
    // Show only the cards for current page
    filteredPenguinCards.slice(startIndex, endIndex).forEach(card => {
        card.style.display = 'block';
    });
    
    // Update page info
    document.getElementById('page-info').textContent = 
        `Page ${currentPage} of ${Math.ceil(filteredPenguinCards.length / cardsPerPage)}`;
    
    // Update button states
    document.getElementById('prev-page').disabled = currentPage === 1;
    document.getElementById('next-page').disabled = 
        endIndex >= filteredPenguinCards.length;
}

function prevPage() {
    if (currentPage > 1) {
        currentPage--;
        updatePage();
    }
}

function nextPage() {
    const totalPages = Math.ceil(filteredPenguinCards.length / cardsPerPage);
    if (currentPage < totalPages) {
        currentPage++;
        updatePage();
    }
}

// Modify your existing filterPenguins function to reset to page 1 when filtering
function filterPenguins() {
    // Your existing filtering logic...
    
    // After filtering, reset to page 1
    currentPage = 1;
    filteredPenguinCards = Array.from(document.querySelectorAll('.penguin-card:not([style*="display: none"])'));
    updatePage();
}