let currentPage = 1;
const cardsPerPage = 15;
let allPenguinCards = [];
let filteredPenguinCards = [];

function filterPenguins() {
    const searchText = document.getElementById('id-search').value.toLowerCase();
    const selectedLocation = document.getElementById('location-filter').value;
    const selectedWeight = document.getElementById('weight-filter').value;
    
    allPenguinCards = document.querySelectorAll('.penguin-card');
    filteredPenguinCards = [];

    allPenguinCards.forEach(card => {
        const penguinId = card.querySelector('h3').textContent.toLowerCase();
        const location = card.querySelector('p:nth-of-type(1)').textContent.toLowerCase();
        const weightText = card.querySelector('p:nth-of-type(2)').textContent;
        const weight = parseFloat(weightText.match(/\d+\.?\d*/)[0]);

        const matchesSearch = searchText === '' || penguinId.includes(searchText);
        const matchesLocation = selectedLocation === 'all' || location.includes(selectedLocation.toLowerCase());
        
        let matchesWeight = true;
        if (selectedWeight !== 'all') {
            if (selectedWeight === 'overweight' && weight <= 3.6) matchesWeight = false;
            if (selectedWeight === 'underweight' && weight >= 2.1) matchesWeight = false;
            if (selectedWeight === 'normal' && (weight < 2.2 || weight > 3.5)) matchesWeight = false;
        }

        if (matchesSearch && matchesLocation && matchesWeight) {
            filteredPenguinCards.push(card);
        }
    });

    currentPage = 1;
    updatePage();
}

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

document.addEventListener('DOMContentLoaded', function() {
    allPenguinCards = document.querySelectorAll('.penguin-card');
    filteredPenguinCards = Array.from(allPenguinCards);
    
    document.getElementById('id-search').addEventListener('input', filterPenguins);
    document.getElementById('location-filter').addEventListener('change', filterPenguins);
    document.getElementById('weight-filter').addEventListener('change', filterPenguins);
    
    updatePage();
});