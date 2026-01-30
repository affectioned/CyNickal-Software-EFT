import sqlite3
import requests
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(SCRIPT_DIR, 'EFT_Data.db')

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
    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()
    result = run_query(ItemQuery)
    for Item in result['data']['items']:

        HighestTraderPrice = -1
            
        for SellOption in Item['sellFor']:
            price = SellOption['priceRUB']
            vendor = SellOption["vendor"]["name"]
            if(vendor == "Flea Market"):
                continue

            if(price > HighestTraderPrice):
                HighestTraderPrice = price

        cur.execute("INSERT OR IGNORE INTO item_data (bsg_id, short_name, trader_price) VALUES (?, ?, ?)", (Item['id'], Item['shortName'], HighestTraderPrice))

    con.commit()
    con.close()
    return

ContainerQuery = """
{
  lootContainers {
    id
    name
  }
}
"""
def UpdateContainerTable():
    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()

    result = run_query(ContainerQuery)

    for Item in result['data']['lootContainers']:
       cur.execute("INSERT OR IGNORE INTO container_data (bsg_id, short_name) VALUES (?, ?)", (Item['id'], Item['name']))

    con.commit()
    con.close()
    return

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
    result = run_query(AmmoQuery)

    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()

    for Item in result['data']['ammo']:
         cur.execute("INSERT OR IGNORE INTO ammo_data (bsg_id, short_name) VALUES (?, ?)", (Item['item']['id'], Item['item']['shortName']))

    con.commit()
    con.close()
    return

ExfilQuery = """
{
  maps {
    nameId
    extracts {
      name
      position {
        x
        y
        z
      }
    }
  }
}
"""
def UpdateExfilTable():
    """Fetch exfil data from tarkov.dev and update the database with positions"""
    
    MAP_NAME_MAPPING = {
        'Woods': ['woods', 'Woods'],
        'Shoreline': ['shoreline', 'Shoreline'],
        'RezervBase': ['rezervbase', 'RezervBase'],
        'Labyrinth': ['Labyrinth'],
        'laboratory': ['laboratory'],
        'Interchange': ['interchange', 'Interchange'],
        'factory4_day': ['factory4_day'],
        'factory4_night': ['factory4_night'],
        'bigmap': ['bigmap'],
        'Lighthouse': ['lighthouse', 'Lighthouse'],
        'TarkovStreets': ['tarkovstreets', 'TarkovStreets'],
        'Sandbox': ['Sandbox'],
        'Sandbox_high': ['Sandbox_high'],
        'Terminal': ['Terminal'],
        'Sandbox_start': ['Sandbox_start'],
    }
    
    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()
    
    result = run_query(ExfilQuery)
    
    total_inserted = 0
    for map_data in result['data']['maps']:
        map_api_id = map_data['nameId']
        internal_names = MAP_NAME_MAPPING.get(map_api_id, [])
        
        if not internal_names:
            continue
        
        for extract in map_data['extracts']:
            pos = extract.get('position', {})
            pos_x = pos.get('x', 0.0)
            pos_y = pos.get('y', 0.0)
            pos_z = pos.get('z', 0.0)
            
            for internal_name in internal_names:
                cur.execute("""
                    INSERT OR REPLACE INTO exfil_data 
                    (map_internal_name, exfil_display_name, pos_x, pos_y, pos_z) 
                    VALUES (?, ?, ?, ?, ?)
                """, (internal_name, extract['name'], pos_x, pos_y, pos_z))
                total_inserted += 1
    
    con.commit()
    con.close()

if __name__ == "__main__":
    print(f"Using database: {DB_PATH}")
    UpdateAmmoTable()
    UpdateItemTable()
    UpdateContainerTable()
    UpdateExfilTable()