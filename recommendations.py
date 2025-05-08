import sys
import argparse
from datetime import datetime

def calculate_age(birthdate_str):
    """Calculates age from MM/DD/YYYY string."""
    try:
        birthdate = datetime.strptime(birthdate_str, "%m/%d/%Y")
        today = datetime.today()
        age = today.year - birthdate.year - ((today.month, today.day) < (birthdate.month, birthdate.day))
        return age
    except ValueError:
        return None # Handle invalid date format

def parse_network_file(filename="networkDB.txt"):
    """Parses the C++ network database file."""
    people = {}
    try:
        with open(filename, 'r') as f:
            current_person = None
            reading_state = 'fname' # fname, lname, bdate, phone, email, info, friends, separator

            lines = f.readlines() # Read all lines at once
            i = 0
            while i < len(lines):
                line = lines[i].strip()
                i += 1

                if reading_state == 'fname':
                    current_person = {'fname': line, 'additional_info': {}, 'friends': []}
                    reading_state = 'lname'
                elif reading_state == 'lname':
                    current_person['lname'] = line
                    # Generate codeName early for map key
                    fname_lower = current_person['fname'].lower().replace(' ', '')
                    lname_lower = current_person['lname'].lower().replace(' ', '')
                    current_person['codeName'] = fname_lower + lname_lower
                    reading_state = 'bdate'
                elif reading_state == 'bdate':
                    current_person['bdate'] = line
                    current_person['age'] = calculate_age(line)
                    reading_state = 'phone'
                elif reading_state == 'phone':
                    current_person['phone'] = line
                    reading_state = 'email'
                elif reading_state == 'email':
                    current_person['email'] = line
                    reading_state = 'info' # Start looking for info or info_end
                elif reading_state == 'info':
                    if line == '---INFO_END---':
                        reading_state = 'friends'
                    elif ':' in line:
                        key, value = line.split(':', 1)
                        current_person['additional_info'][key.strip()] = value.strip()
                    elif line == '--------------------': # Old separator case
                         if current_person:
                             people[current_person['codeName']] = current_person
                         reading_state = 'fname'
                    else: # Assume it's a friend code if not info/separator/end
                         current_person['friends'].append(line)
                         reading_state = 'friends' # Switch to friend reading state
                elif reading_state == 'friends':
                    if line == '--------------------':
                        if current_person:
                            people[current_person['codeName']] = current_person
                        reading_state = 'fname'
                    else:
                        current_person['friends'].append(line)

            # Add the last person if file doesn't end with separator
            if current_person and reading_state != 'fname' and current_person['codeName'] not in people:
                 people[current_person['codeName']] = current_person

    except FileNotFoundError:
        print(f"Error: Database file '{filename}' not found.", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Error parsing file '{filename}': {e}", file=sys.stderr)
        return None
    return people

def recommend_friends(target_codename, people_data, max_recommendations=5):
    """Recommends friends based on shared info and age proximity."""
    if target_codename not in people_data:
        print(f"Error: Person with codeName '{target_codename}' not found.", file=sys.stderr)
        return []

    target_person = people_data[target_codename]
    recommendations = {}

    target_info = target_person.get('additional_info', {})
    target_friends = set(target_person.get('friends', []))
    target_age = target_person.get('age')

    for code, person in people_data.items():
        if code == target_codename or code in target_friends:
            continue # Skip self and existing friends

        score = 0
        person_info = person.get('additional_info', {})
        person_age = person.get('age')

        # Score based on shared additional info (simple count)
        shared_keys = set(target_info.keys()) & set(person_info.keys())
        for key in shared_keys:
            if target_info[key] == person_info[key]:
                score += 2 # Higher score for matching values
            else:
                score += 1 # Lower score for just sharing a key

        # Score based on age proximity (optional)
        if target_age is not None and person_age is not None:
            age_diff = abs(target_age - person_age)
            if age_diff <= 5:
                score += 1 # Bonus for similar age

        if score > 0:
            recommendations[code] = score

    # Sort by score descending
    sorted_recommendations = sorted(recommendations.items(), key=lambda item: item[1], reverse=True)

    # Return top N codenames
    return [code for code, score in sorted_recommendations[:max_recommendations]]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Recommend friends based on TrojanBook network data.")
    parser.add_argument("codename", help="The codeName of the person to get recommendations for.")
    parser.add_argument("-f", "--file", default="networkDB.txt", help="Path to the network database file.")
    parser.add_argument("-n", "--num", type=int, default=5, help="Maximum number of recommendations to return.")

    args = parser.parse_args()

    people = parse_network_file(args.file)

    if people:
        recommended_codes = recommend_friends(args.codename, people, args.num)
        if recommended_codes:
            # Print one code per line for easy parsing by Node.js
            for code in recommended_codes:
                print(code) 