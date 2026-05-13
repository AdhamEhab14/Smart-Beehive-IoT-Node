import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(const HiveApp());
}

class HiveApp extends StatelessWidget {
  const HiveApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Smart Hive',
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF121212),
        primaryColor: Colors.amber,
      ),
      home: const HiveDashboard(),
    );
  }
}

class HiveDashboard extends StatefulWidget {
  const HiveDashboard({super.key});

  @override
  State<HiveDashboard> createState() => _HiveDashboardState();
}

class _HiveDashboardState extends State<HiveDashboard> {
  final DatabaseReference _hiveRef = FirebaseDatabase.instance.ref('hive');

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text(
          'Hive Telemetry',
          style: TextStyle(fontWeight: FontWeight.bold),
        ),
        backgroundColor: Colors.black,
        elevation: 0,
      ),
      body: StreamBuilder(
        stream: _hiveRef.onValue,
        builder: (context, AsyncSnapshot<DatabaseEvent> snapshot) {
          if (snapshot.hasError) {
            return const Center(child: Text('Connection Error'));
          }
          if (!snapshot.hasData || snapshot.data!.snapshot.value == null) {
            return const Center(
              child: CircularProgressIndicator(color: Colors.amber),
            );
          }

          // Extract the data from the cloud
          final data = Map<String, dynamic>.from(
            snapshot.data!.snapshot.value as Map,
          );
          final double temp = (data['temperature'] ?? 0.0).toDouble();
          final double humidity = (data['humidity'] ?? 0.0).toDouble();
          final bool fanStatus = data['manual_fan'] ?? false;

          return Padding(
            padding: const EdgeInsets.all(24.0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                // Gauges Row with Expanded to prevent overflow
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    // Temperature Gauge
                    Expanded(
                      child: _buildGauge(
                        label: 'Temperature',
                        icon: Icons.thermostat,
                        value: '${temp.toStringAsFixed(1)}°C',
                        color: temp >= 28.0 ? Colors.redAccent : Colors.amber,
                      ),
                    ),
                    // Humidity Gauge
                    Expanded(
                      child: _buildGauge(
                        label: 'Humidity',
                        icon: Icons.water_drop,
                        value: '${humidity.toStringAsFixed(1)}%',
                        color: Colors.lightBlueAccent,
                      ),
                    ),
                  ],
                ),

                const SizedBox(height: 60),

                // Manual Fan Switch
                Container(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 20,
                    vertical: 10,
                  ),
                  decoration: BoxDecoration(
                    color: Colors.grey[900],
                    borderRadius: BorderRadius.circular(15),
                  ),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      const Row(
                        children: [
                          Icon(Icons.cyclone, color: Colors.lightBlueAccent),
                          SizedBox(width: 15),
                          Text(
                            'Cooling Fan',
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.w500,
                            ),
                          ),
                        ],
                      ),
                      Switch(
                        value: fanStatus,
                        activeThumbColor: Colors.lightBlueAccent,
                        onChanged: (bool newValue) {
                          _hiveRef.update({'manual_fan': newValue});
                        },
                      ),
                    ],
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }

  Widget _buildGauge({
    required String label,
    required IconData icon,
    required String value,
    required Color color,
  }) {
    return Column(
      children: [
        Container(
          padding: const EdgeInsets.all(25),
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            border: Border.all(color: color, width: 4),
            boxShadow: [
              BoxShadow(
                color: color.withOpacity(0.15),
                blurRadius: 20,
                spreadRadius: 5,
              ),
            ],
          ),
          child: Column(
            children: [
              Icon(icon, size: 35, color: Colors.white70),
              const SizedBox(height: 10),
              FittedBox(
                fit: BoxFit.scaleDown,
                child: Text(
                  value,
                  style: const TextStyle(
                    fontSize: 32,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ],
          ),
        ),
        const SizedBox(height: 15),
        Text(
          label,
          style: const TextStyle(
            fontSize: 16,
            color: Colors.white70,
            fontWeight: FontWeight.w600,
            letterSpacing: 1.2,
          ),
        ),
      ],
    );
  }
}
