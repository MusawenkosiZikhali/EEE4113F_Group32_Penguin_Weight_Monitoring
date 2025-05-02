document.addEventListener('DOMContentLoaded', function() {
    // Get the canvas element
    const ctx = document.getElementById('weightChart');
    
    // Create the chart with sample data
    new Chart(ctx, {
        type: 'bar',
        data: {
            labels: ['Adelie', 'Gentoo', 'Chinstrap'],
            datasets: [{
                label: 'Average Weight (kg)',
                data: [4.2, 5.1, 3.8],
                backgroundColor: [
                    '#3a86ff',
                    '#8338ec',
                    '#ff006e'
                ],
                borderColor: [
                    '#2a75e6',
                    '#7328d4',
                    '#e6005e'
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: 'Penguin Weights by Species'
                }
            },
            scales: {
                y: {
                    beginAtZero: false,
                    min: 3,
                    title: {
                        display: true,
                        text: 'Weight (kg)'
                    }
                }
            }
        }
    });

    // Demo buttons
    document.getElementById('apply-filters').addEventListener('click', function() {
        console.log('Filters applied (demo)');
    });
});