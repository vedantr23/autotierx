import { useCallback, useEffect, useState } from "react";
import axios from "axios";
import {
    PieChart,
    Pie,
    Cell,
    Tooltip,
    BarChart,
    Bar,
    XAxis,
    YAxis,
    CartesianGrid,
    ResponsiveContainer
} from "recharts";
import "./App.css";

function App() {
    const [tiers, setTiers] = useState([]);
    const [objects, setObjects] = useState([]);
    const [metrics, setMetrics] = useState({});
    const [selectedFile, setSelectedFile] = useState(null);
    const [statusMessage, setStatusMessage] = useState("");

    const tierChartData = tiers.map((tier) => ({
        name: tier.name,
        capacity: tier.capacity
    }));

    const objectDistribution = {};
    objects.forEach((object) => {
        const currentTier = object.tier || "Unknown";
        objectDistribution[currentTier] =
            (objectDistribution[currentTier] || 0) + 1;
    });

    const objectChartData = Object.entries(objectDistribution).map(
        ([tier, count]) => ({ tier, count })
    );

    const API_BASE = "http://localhost:18080";

    const COLORS = ["#3b82f6", "#22c55e", "#eab308", "#ef4444"];

    const normalizeResponse = useCallback(
        (data) => (Array.isArray(data) ? data : Object.values(data || {})),
        []
    );

    const fetchTiers = useCallback(async () => {
        try {
            const response = await axios.get(`${API_BASE}/tiers`);
            setTiers(normalizeResponse(response.data));
        } catch (error) {
            console.error("Failed to load tiers", error);
            setStatusMessage("Unable to load storage tiers.");
        }
    }, [API_BASE, normalizeResponse]);

    const fetchObjects = useCallback(async () => {
        try {
            const response = await axios.get(`${API_BASE}/objects`);
            setObjects(normalizeResponse(response.data));
        } catch (error) {
            console.error("Failed to load objects", error);
            setStatusMessage("Unable to load stored objects.");
        }
    }, [API_BASE, normalizeResponse]);

    const fetchMetrics = useCallback(async () => {
        try {
            const response = await axios.get(`${API_BASE}/metrics`);
            setMetrics(response.data || {});
        } catch (error) {
            console.error("Failed to load metrics", error);
            setStatusMessage("Unable to load metrics.");
        }
    }, [API_BASE]);

    const refreshDashboard = useCallback(async () => {
        await Promise.all([fetchTiers(), fetchObjects(), fetchMetrics()]);
    }, [fetchTiers, fetchObjects, fetchMetrics]);

    useEffect(() => {
        const loadDashboard = async () => {
            await refreshDashboard();
        };

        void loadDashboard();
        const interval = setInterval(() => {
            void refreshDashboard();
        }, 2500);

        return () => clearInterval(interval);
    }, [refreshDashboard]);

    const handleFileChange = (event) => {
        setSelectedFile(event.target.files[0]);
    };

    const uploadFile = async () => {
        if (!selectedFile) {
            setStatusMessage("Select a file before uploading.");
            return;
        }

        try {
            const response = await fetch(`${API_BASE}/upload`, {
                method: "POST",
                headers: {
                    "X-Filename": selectedFile.name,
                    "Content-Type": "application/octet-stream"
                },
                body: await selectedFile.arrayBuffer()
            });

            const result = await response.json();
            if (!response.ok) {
                throw new Error(result.message || "Upload failed");
            }

            setStatusMessage(`Uploaded ${selectedFile.name} successfully.`);
            setSelectedFile(null);
            await refreshDashboard();
        } catch (error) {
            console.error("Upload error", error);
            setStatusMessage(`Upload failed: ${error.message}`);
        }
    };

    const downloadFile = async (filename) => {
        try {
            window.open(
                `${API_BASE}/download/${encodeURIComponent(filename)}`,
                "_blank"
            );
        } catch (error) {
            console.error("Download error", error);
            setStatusMessage(`Download failed for ${filename}.`);
        }
    };

    const deleteFile = async (filename) => {
        try {
            const response = await fetch(
                `${API_BASE}/delete/${encodeURIComponent(filename)}`,
                { method: "DELETE" }
            );

            const result = await response.json();
            if (!response.ok) {
                throw new Error(result.message || "Delete failed");
            }

            setStatusMessage(`Deleted ${filename} successfully.`);
            await refreshDashboard();
        } catch (error) {
            console.error("Delete error", error);
            setStatusMessage(`Delete failed: ${error.message}`);
        }
    };

    return (
        <div className="app-shell">
            <header className="dashboard-header">
                <div className="header-copy">
                    <p className="eyebrow">AutoTierX</p>
                    <h1>Tiered Storage Control Center</h1>
                    <p className="description">
                        Real-time file management, analytics, and storage tier visibility for your object storage platform.
                    </p>
                </div>
                <div className="actions-row">
                    <label className="file-input-label">
                        <span>{selectedFile ? selectedFile.name : "Select a file"}</span>
                        <input type="file" accept="*/*" onChange={handleFileChange} />
                    </label>
                    <button className="button button-primary" onClick={uploadFile}>
                        Upload File
                    </button>
                    <button className="button button-secondary" onClick={refreshDashboard}>
                        Refresh
                    </button>
                </div>
            </header>

            {statusMessage && <div className="status-banner">{statusMessage}</div>}

            <section className="metrics-grid">
                <article className="metric-card">
                    <span className="metric-label">Total Tiers</span>
                    <p className="metric-value">{metrics.total_tiers ?? 0}</p>
                </article>
                <article className="metric-card">
                    <span className="metric-label">Online Tiers</span>
                    <p className="metric-value">{metrics.online_tiers ?? 0}</p>
                </article>
                <article className="metric-card">
                    <span className="metric-label">Total Capacity</span>
                    <p className="metric-value">{metrics.total_capacity_gb ?? 0} GB</p>
                </article>
            </section>

            <section className="charts-row">
                <div className="chart-card">
                    <div className="chart-card-header">
                        <h2>Tier Capacity</h2>
                    </div>
                    <ResponsiveContainer width="100%" height={320}>
                        <PieChart>
                            <Pie
                                data={tierChartData}
                                dataKey="capacity"
                                nameKey="name"
                                outerRadius={110}
                                label
                            >
                                {tierChartData.map((entry, index) => (
                                    <Cell key={index} fill={COLORS[index % COLORS.length]} />
                                ))}
                            </Pie>
                            <Tooltip />
                        </PieChart>
                    </ResponsiveContainer>
                </div>
                <div className="chart-card">
                    <div className="chart-card-header">
                        <h2>Object Distribution</h2>
                    </div>
                    <ResponsiveContainer width="100%" height={320}>
                        <BarChart data={objectChartData}>
                            <CartesianGrid strokeDasharray="3 3" />
                            <XAxis dataKey="tier" />
                            <YAxis />
                            <Tooltip />
                            <Bar dataKey="count" fill="#3b82f6" />
                        </BarChart>
                    </ResponsiveContainer>
                </div>
            </section>

            <section className="section-panel">
                <div className="section-header">
                    <div>
                        <p className="eyebrow">Storage Tiers</p>
                        <h2>Live Tier Inventory</h2>
                    </div>
                </div>
                <div className="panel-grid">
                    {tiers.map((tier, index) => (
                        <article key={index} className="panel-card">
                            <span className="panel-title">{tier.name}</span>
                            <p>
                                <strong>Status:</strong> {tier.status}
                            </p>
                            <p>
                                <strong>Capacity:</strong> {tier.capacity} GB
                            </p>
                            <p className="panel-path">{tier.path}</p>
                        </article>
                    ))}
                </div>
            </section>

            <section className="section-panel">
                <div className="section-header">
                    <div>
                        <p className="eyebrow">Stored Objects</p>
                        <h2>Object Inventory</h2>
                    </div>
                </div>
                <div className="panel-grid">
                    {objects.map((object, index) => (
                        <article key={index} className="panel-card object-card">
                            <span className="panel-title">{object.filename}</span>
                            <p>
                                <strong>Tier:</strong> {object.tier}
                            </p>
                            <p>
                                <strong>Size:</strong> {object.size_bytes} bytes
                            </p>
                            <p>
                                <strong>Access Count:</strong> {object.access_count}
                            </p>
                            <p className="panel-meta">{object.last_accessed}</p>
                            <div className="button-group">
                                <button
                                    className="button button-primary"
                                    onClick={() => downloadFile(object.filename)}
                                >
                                    Download
                                </button>
                                <button
                                    className="button button-danger"
                                    onClick={() => deleteFile(object.filename)}
                                >
                                    Delete
                                </button>
                            </div>
                        </article>
                    ))}
                </div>
            </section>
        </div>
    );
}

export default App;
