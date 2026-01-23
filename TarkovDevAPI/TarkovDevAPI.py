import sqlite3
import requests

def run_query(query):
    headers = {"Content-Type": "application/json"}
    response = requests.post('https://api.tarkov.dev/graphql', headers=headers, json={'query': query})
    if response.status_code == 200:
        return response.json()
    else:
        raise Exception("Query failed to run by returning code of {}. {}".format(response.status_code, query))

ItemQuery = """
{
  items(lang: en) {
    id
    sellFor {
      priceRUB
      vendor {
        name
      }
    }
    shortName
  }
}
"""
def UpdateItemTable():
    con = sqlite3.connect('../CyNickal Software EFT/EFT_Data.db');
    cur = con.cursor();
    result = run_query(ItemQuery);
    for Item in result['data']['items']:

        HighestTraderPrice = -1;
            
        for SellOption in Item['sellFor']:
            price = SellOption['priceRUB'];
            vendor = SellOption["vendor"]["name"];
            if(vendor == "Flea Market"):
                continue;

            if(price > HighestTraderPrice):
                HighestTraderPrice = price;

        cur.execute("INSERT OR IGNORE INTO item_data (bsg_id, short_name, trader_price) VALUES (?, ?, ?)", (Item['id'], Item['shortName'], HighestTraderPrice));

    con.commit();
    con.close();
    return;

ContainerQuery = """
{
  lootContainers {
    id
    name
  }
}
"""
def UpdateContainerTable():
    con = sqlite3.connect('../CyNickal Software EFT/EFT_Data.db');
    cur = con.cursor();

    result = run_query(ContainerQuery);

    for Item in result['data']['lootContainers']:
       cur.execute("INSERT OR IGNORE INTO container_data (bsg_id, short_name) VALUES (?, ?)", (Item['id'], Item['name']));

    con.commit();
    con.close();
    return;

AmmoQuery ="""
{
  ammo {
    item {
      id
      shortName
    }
  }
}
"""
def UpdateAmmoTable():
    result = run_query(AmmoQuery);

    con = sqlite3.connect('../CyNickal Software EFT/EFT_Data.db');
    cur = con.cursor();

    for Item in result['data']['ammo']:
         cur.execute("INSERT OR IGNORE INTO ammo_data (bsg_id, short_name) VALUES (?, ?)", (Item['item']['id'], Item['item']['shortName']));

    con.commit();
    con.close();
    return;

# NEW: Exfil Query
ExfilQuery = """
{
  maps {
    name
    nameId
    extracts {
      name
      faction
    }
  }
}
"""

def CreateExfilTable():
    """Create the exfil_data table if it doesn't exist"""
    con = sqlite3.connect('../CyNickal Software EFT/EFT_Data.db')
    cur = con.cursor()
    
    cur.execute("""
        CREATE TABLE IF NOT EXISTS exfil_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            map_internal_name TEXT NOT NULL,
            map_display_name TEXT NOT NULL,
            exfil_internal_name TEXT NOT NULL,
            exfil_display_name TEXT NOT NULL,
            faction TEXT,
            UNIQUE(map_internal_name, exfil_internal_name)
        )
    """)
    
    # Create index for fast lookups
    cur.execute("""
        CREATE INDEX IF NOT EXISTS idx_exfil_lookup 
        ON exfil_data(map_internal_name, exfil_internal_name)
    """)
    
    con.commit()
    con.close()

def UpdateExfilTable():
    """Fetch exfil data from tarkov.dev and update the database"""
    
    MAP_NAME_MAPPING = {
        'woods': 'woods',
        'shoreline': 'shoreline',
        'rezervbase': 'reserve',
        'Labyrinth': 'labs',
        'laboratory': 'labs',
        'interchange': 'interchange',
        'factory4_day': 'factory',
        'factory4_night': 'factory',
        'bigmap': 'customs',
        'lighthouse': 'lighthouse',
        'tarkovstreets': 'streets-of-tarkov',
        'Sandbox': 'ground-zero',
        'Sandbox_high': 'ground-zero'
    }
    
    con = sqlite3.connect('../CyNickal Software EFT/EFT_Data.db')
    cur = con.cursor()
    
    result = run_query(ExfilQuery)
    
    reverse_mapping = {}
    for internal, api_id in MAP_NAME_MAPPING.items():
        if api_id not in reverse_mapping:
            reverse_mapping[api_id] = []
        reverse_mapping[api_id].append(internal)
    
    total_inserted = 0
    for map_data in result['data']['maps']:
        map_api_id = map_data['nameId']
        map_display_name = map_data['name']
        
        internal_names = reverse_mapping.get(map_api_id, [])
        
        if not internal_names:
            print(f"Warning: No internal name mapping for {map_api_id}")
            continue
        
        for extract in map_data['extracts']:
            exfil_name = extract['name']
            faction = extract.get('faction', 'Unknown')
            
            # Insert for each internal map name (e.g., both factory4_day and factory4_night)
            for internal_name in internal_names:
                cur.execute("""
                    INSERT OR REPLACE INTO exfil_data 
                    (map_internal_name, map_display_name, exfil_internal_name, exfil_display_name, faction) 
                    VALUES (?, ?, ?, ?, ?)
                """, (internal_name, map_display_name, exfil_name, exfil_name, faction))
                total_inserted += 1
        
        print(f"Updated {len(map_data['extracts'])} exfils for {map_display_name}")
    
    con.commit()
    con.close()
    print(f"Exfil data updated successfully! Total entries: {total_inserted}")

if __name__ == "__main__":
    UpdateAmmoTable()
    UpdateItemTable()
    UpdateContainerTable()
    CreateExfilTable()
    UpdateExfilTable()