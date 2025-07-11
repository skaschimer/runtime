// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections;
using System.Collections.Generic;
using System.Text.Json.Serialization.Converters;
using System.Threading;

namespace System.Text.Json.Nodes
{
    public partial class JsonObject : IDictionary<string, JsonNode?>
    {
        private OrderedDictionary<string, JsonNode?>? _dictionary;

        /// <summary>
        ///   Adds an element with the provided property name and value to the <see cref="JsonObject"/>.
        /// </summary>
        /// <param name="propertyName">The property name of the element to add.</param>
        /// <param name="value">The value of the element to add.</param>
        /// <exception cref="ArgumentNullException">
        ///   <paramref name="propertyName"/>is <see langword="null"/>.
        /// </exception>
        /// <exception cref="ArgumentException">
        ///   An element with the same property name already exists in the <see cref="JsonObject"/>.
        /// </exception>
        public void Add(string propertyName, JsonNode? value)
        {
            ArgumentNullException.ThrowIfNull(propertyName);

            Dictionary.Add(propertyName, value);
            value?.AssignParent(this);
        }

        /// <summary>
        ///   Adds an element with the provided name and value to the <see cref="JsonObject"/>, if a property named <paramref name="propertyName"/> doesn't already exist.
        /// </summary>
        /// <param name="propertyName">The property name of the element to add.</param>
        /// <param name="value">The value of the element to add.</param>
        /// <exception cref="ArgumentNullException"><paramref name="propertyName"/> is null.</exception>
        /// <returns>
        ///   <see langword="true"/> if the property didn't exist and the element was added; otherwise, <see langword="false"/>.
        /// </returns>
        public bool TryAdd(string propertyName, JsonNode? value) => TryAdd(propertyName, value, out _);

        /// <summary>
        ///   Adds an element with the provided name and value to the <see cref="JsonObject"/>, if a property named <paramref name="propertyName"/> doesn't already exist.
        /// </summary>
        /// <param name="propertyName">The property name of the element to add.</param>
        /// <param name="value">The value of the element to add.</param>
        /// <param name="index">The index of the added or existing <paramref name="propertyName"/>. This is always a valid index into the <see cref="JsonObject"/>.</param>
        /// <exception cref="ArgumentNullException"><paramref name="propertyName"/> is null.</exception>
        /// <returns>
        ///   <see langword="true"/> if the property didn't exist and the element was added; otherwise, <see langword="false"/>.
        /// </returns>
        public bool TryAdd(string propertyName, JsonNode? value, out int index)
        {
            if (propertyName is null)
            {
                ThrowHelper.ThrowArgumentNullException(nameof(propertyName));
            }
#if NET9_0
            bool success = Dictionary.TryAdd(propertyName, value);
            index = success ? Dictionary.Count - 1 : Dictionary.IndexOf(propertyName);
#else
            bool success = Dictionary.TryAdd(propertyName, value, out index);
#endif
            if (success)
            {
                value?.AssignParent(this);
            }
            return success;
        }

        /// <summary>
        ///   Adds the specified property to the <see cref="JsonObject"/>.
        /// </summary>
        /// <param name="property">
        ///   The KeyValuePair structure representing the property name and value to add to the <see cref="JsonObject"/>.
        /// </param>
        /// <exception cref="ArgumentException">
        ///   An element with the same property name already exists in the <see cref="JsonObject"/>.
        /// </exception>
        /// <exception cref="ArgumentNullException">
        ///   The property name of <paramref name="property"/> is <see langword="null"/>.
        /// </exception>
        public void Add(KeyValuePair<string, JsonNode?> property) => Add(property.Key, property.Value);

        /// <summary>
        ///   Removes all elements from the <see cref="JsonObject"/>.
        /// </summary>
        public void Clear()
        {
            OrderedDictionary<string, JsonNode?>? dictionary = _dictionary;

            if (dictionary is null)
            {
                _jsonElement = null;
                return;
            }

            foreach (JsonNode? node in dictionary.Values)
            {
                DetachParent(node);
            }

            dictionary.Clear();
        }

        /// <summary>
        ///   Determines whether the <see cref="JsonObject"/> contains an element with the specified property name.
        /// </summary>
        /// <param name="propertyName">The property name to locate in the <see cref="JsonObject"/>.</param>
        /// <returns>
        ///   <see langword="true"/> if the <see cref="JsonObject"/> contains an element with the specified property name; otherwise, <see langword="false"/>.
        /// </returns>
        /// <exception cref="ArgumentNullException">
        ///   <paramref name="propertyName"/> is <see langword="null"/>.
        /// </exception>
        public bool ContainsKey(string propertyName)
        {
            ArgumentNullException.ThrowIfNull(propertyName);

            return Dictionary.ContainsKey(propertyName);
        }

        /// <summary>
        ///   Gets the number of elements contained in <see cref="JsonObject"/>.
        /// </summary>
        public int Count => Dictionary.Count;

        /// <summary>
        ///   Removes the element with the specified property name from the <see cref="JsonObject"/>.
        /// </summary>
        /// <param name="propertyName">The property name of the element to remove.</param>
        /// <returns>
        ///   <see langword="true"/> if the element is successfully removed; otherwise, <see langword="false"/>.
        /// </returns>
        /// <exception cref="ArgumentNullException">
        ///   <paramref name="propertyName"/> is <see langword="null"/>.
        /// </exception>
        public bool Remove(string propertyName)
        {
            ArgumentNullException.ThrowIfNull(propertyName);

            bool success = Dictionary.Remove(propertyName, out JsonNode? removedNode);
            if (success)
            {
                DetachParent(removedNode);
            }

            return success;
        }

        /// <summary>
        ///   Determines whether the <see cref="JsonObject"/> contains a specific property name and <see cref="JsonNode"/> reference.
        /// </summary>
        /// <param name="item">The element to locate in the <see cref="JsonObject"/>.</param>
        /// <returns>
        ///   <see langword="true"/> if the <see cref="JsonObject"/> contains an element with the property name; otherwise, <see langword="false"/>.
        /// </returns>
        bool ICollection<KeyValuePair<string, JsonNode?>>.Contains(KeyValuePair<string, JsonNode?> item) =>
            ((IDictionary<string, JsonNode?>)Dictionary).Contains(item);

        /// <summary>
        ///   Copies the elements of the <see cref="JsonObject"/> to an array of type KeyValuePair starting at the specified array index.
        /// </summary>
        /// <param name="array">
        ///   The one-dimensional Array that is the destination of the elements copied from <see cref="JsonObject"/>.
        /// </param>
        /// <param name="index">The zero-based index in <paramref name="array"/> at which copying begins.</param>
        /// <exception cref="ArgumentNullException">
        ///   <paramref name="array"/> is <see langword="null"/>.
        /// </exception>
        /// <exception cref="ArgumentOutOfRangeException">
        ///   <paramref name="index"/> is less than 0.
        /// </exception>
        /// <exception cref="ArgumentException">
        ///   The number of elements in the source ICollection is greater than the available space from <paramref name="index"/>
        ///   to the end of the destination <paramref name="array"/>.
        /// </exception>
        void ICollection<KeyValuePair<string, JsonNode?>>.CopyTo(KeyValuePair<string, JsonNode?>[] array, int index) =>
            ((IDictionary<string, JsonNode?>)Dictionary).CopyTo(array, index);

        /// <summary>
        ///   Returns an enumerator that iterates through the <see cref="JsonObject"/>.
        /// </summary>
        /// <returns>
        ///   An enumerator that iterates through the <see cref="JsonObject"/>.
        /// </returns>
        public IEnumerator<KeyValuePair<string, JsonNode?>> GetEnumerator() => Dictionary.GetEnumerator();

        /// <summary>
        ///   Removes a key and value from the <see cref="JsonObject"/>.
        /// </summary>
        /// <param name="item">
        ///   The KeyValuePair structure representing the property name and value to remove from the <see cref="JsonObject"/>.
        /// </param>
        /// <returns>
        ///   <see langword="true"/> if the element is successfully removed; otherwise, <see langword="false"/>.
        /// </returns>
        bool ICollection<KeyValuePair<string, JsonNode?>>.Remove(KeyValuePair<string, JsonNode?> item) => Remove(item.Key);

        /// <summary>
        ///   Gets a collection containing the property names in the <see cref="JsonObject"/>.
        /// </summary>
        ICollection<string> IDictionary<string, JsonNode?>.Keys => Dictionary.Keys;

        /// <summary>
        ///   Gets a collection containing the property values in the <see cref="JsonObject"/>.
        /// </summary>
        ICollection<JsonNode?> IDictionary<string, JsonNode?>.Values => Dictionary.Values;

        /// <summary>
        ///   Gets the value associated with the specified property name.
        /// </summary>
        /// <param name="propertyName">The property name of the value to get.</param>
        /// <param name="jsonNode">
        ///   When this method returns, contains the value associated with the specified property name, if the property name is found;
        ///   otherwise, <see langword="null"/>.
        /// </param>
        /// <returns>
        ///   <see langword="true"/> if the <see cref="JsonObject"/> contains an element with the specified property name; otherwise, <see langword="false"/>.
        /// </returns>
        /// <exception cref="ArgumentNullException">
        ///   <paramref name="propertyName"/> is <see langword="null"/>.
        /// </exception>
        bool IDictionary<string, JsonNode?>.TryGetValue(string propertyName, out JsonNode? jsonNode)
        {
            ArgumentNullException.ThrowIfNull(propertyName);

            return Dictionary.TryGetValue(propertyName, out jsonNode);
        }

        /// <summary>
        ///   Returns <see langword="false"/>.
        /// </summary>
        bool ICollection<KeyValuePair<string, JsonNode?>>.IsReadOnly => false;

        /// <summary>
        ///   Returns an enumerator that iterates through the <see cref="JsonObject"/>.
        /// </summary>
        /// <returns>
        ///   An enumerator that iterates through the <see cref="JsonObject"/>.
        /// </returns>
        IEnumerator IEnumerable.GetEnumerator() => Dictionary.GetEnumerator();

        private OrderedDictionary<string, JsonNode?> InitializeDictionary()
        {
            GetUnderlyingRepresentation(out OrderedDictionary<string, JsonNode?>? dictionary, out JsonElement? jsonElement);

            if (dictionary is null)
            {
                dictionary = CreateDictionary(Options);

                if (jsonElement.HasValue)
                {
                    foreach (JsonProperty jElementProperty in jsonElement.Value.EnumerateObject())
                    {
                        JsonNode? node = JsonNodeConverter.Create(jElementProperty.Value, Options);
                        if (node != null)
                        {
                            node.Parent = this;
                        }

                        dictionary.Add(jElementProperty.Name, node);
                    }
                }

                // Ensure _jsonElement is written to after _dictionary
                _dictionary = dictionary;
                Interlocked.MemoryBarrier();
                _jsonElement = null;
            }

            return dictionary;
        }

        private static OrderedDictionary<string, JsonNode?> CreateDictionary(JsonNodeOptions? options, int capacity = 0)
        {
            StringComparer comparer = options?.PropertyNameCaseInsensitive ?? false
                ? StringComparer.OrdinalIgnoreCase
                : StringComparer.Ordinal;

            return new(capacity, comparer);
        }

        /// <summary>
        /// Provides a coherent view of the underlying representation of the current node.
        /// The jsonElement value should be consumed if and only if dictionary value is null.
        /// </summary>
        private void GetUnderlyingRepresentation(out OrderedDictionary<string, JsonNode?>? dictionary, out JsonElement? jsonElement)
        {
            // Because JsonElement cannot be read atomically there might be torn reads,
            // however the order of read/write operations guarantees that that's only
            // possible if the value of _dictionary is non-null.
            jsonElement = _jsonElement;
            Interlocked.MemoryBarrier();
            dictionary = _dictionary;
        }
    }
}
