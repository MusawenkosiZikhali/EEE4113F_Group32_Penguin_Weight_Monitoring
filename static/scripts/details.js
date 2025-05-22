document.addEventListener('DOMContentLoaded', function() {
    // Initialize the weight trend chart
    initWeightChart();
    
    // Set up measurement table with pagination
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
// Section 5: Pagination
document.addEventListener('DOMContentLoaded', function() {
  const rowsPerPage = 10;
  const tableBody = document.getElementById('table-body');
  const rows = Array.from(tableBody.querySelectorAll('tr'));
  const totalPages = Math.ceil(rows.length / rowsPerPage);
  let currentPage = 1;

  const prevBtn = document.getElementById('prev-page');
  const nextBtn = document.getElementById('next-page');
  const pageInfo = document.getElementById('page-info');

  function showPage(page) {
    const startRow = (page - 1) * rowsPerPage;
    const endRow = page * rowsPerPage;

    rows.forEach((row, index) => {
      if (index >= startRow && index < endRow) {
        row.style.display = 'table-row';
      } else {
        row.style.display = 'none';
      }
    });

    pageInfo.textContent = `Page ${page} of ${totalPages}`;

    // Enable/Disable buttons
    prevBtn.disabled = page === 1;
    nextBtn.disabled = page === totalPages;
  }

  // Initial page load
  showPage(currentPage);

  // Event listeners for buttons
  prevBtn.addEventListener('click', function() {
    if (currentPage > 1) {
      currentPage--;
      showPage(currentPage);
    }
  });

    nextBtn.addEventListener('click', function() {
    if (currentPage < totalPages) {
      currentPage++;
      showPage(currentPage);
    }
  });
});

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
        'Location': 'Where the measurement was taken'
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

