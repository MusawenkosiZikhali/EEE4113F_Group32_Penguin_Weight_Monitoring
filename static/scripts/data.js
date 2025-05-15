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