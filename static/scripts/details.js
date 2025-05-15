document.addEventListener('DOMContentLoaded', function() {
    // Initialize the weight trend chart
    initWeightChart();
    
    // Set up interactive elements
    setupMeasurementTable();
    
    // Add any dynamic behavior
    setupDynamicUI();
});

function initWeightChart() {
    const ctx = document.getElementById('weightChart').getContext('2d');
    const weightData = {
        labels: JSON.parse('{{ stats.date_labels|tojson }}'),
        datasets: [{
            label: 'Weight (kg)',
            data: JSON.parse('{{ stats.weight_history|tojson }}'),
            borderColor: 'rgba(58, 134, 255, 1)',
            backgroundColor: 'rgba(58, 134, 255, 0.2)',
            borderWidth: 2,
            tension: 0.3,
            fill: true
        }]
    };

    new Chart(ctx, {
        type: 'line',
        data: weightData,
        options: {
            responsive: true,
            plugins: {
                tooltip: {
                    callbacks: {
                        label: function(context) {
                            return `Weight: ${context.parsed.y} kg`;
                        }
                    }
                }
            },
            scales: {
                y: {
                    beginAtZero: false,
                    title: {
                        display: true,
                        text: 'Weight (kg)'
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Measurement Date'
                    }
                }
            }
        }
    });
}

function setupMeasurementTable() {
    const table = document.querySelector('.measurement-table table');
    if (!table) return;
    
    // Make table rows interactive
    table.addEventListener('click', function(e) {
        const row = e.target.closest('tr');
        if (!row || row.classList.contains('expanded')) return;
        
        // Highlight selected measurement
        document.querySelectorAll('tr').forEach(r => r.classList.remove('selected'));
        row.classList.add('selected');
        
        // You could add additional functionality here
        // For example, show more details about this specific measurement
    });
}

function setupDynamicUI() {
    // Add status indicator animation
    const statusBadge = document.querySelector('.status-badge');
    if (statusBadge && statusBadge.classList.contains('status-critical')) {
        setInterval(() => {
            statusBadge.style.opacity = statusBadge.style.opacity === '0.7' ? '1' : '0.7';
        }, 1000);
    }
    
    // Add tooltips to table headers
    const tooltips = {
        'Date': 'Date of measurement (YYYY-MM-DD)',
        'Weight': 'Weight in kilograms',
        'Condition': 'Health assessment status'
    };
    
    document.querySelectorAll('th').forEach(th => {
        const text = th.textContent.trim();
        if (tooltips[text]) {
            th.setAttribute('title', tooltips[text]);
            th.style.cursor = 'help';
        }
    });
    
    // Add responsive table behavior
    window.addEventListener('resize', function() {
        const tableContainer = document.querySelector('.measurement-table');
        if (window.innerWidth < 768) {
            tableContainer.classList.add('mobile-view');
        } else {
            tableContainer.classList.remove('mobile-view');
        }
    });
}