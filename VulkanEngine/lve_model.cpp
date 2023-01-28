#include "lve_model.h"
#include "lve_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERINENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

namespace std {
	template<>
	struct hash<lve::LveModel::Vertex> {
		size_t operator() (lve::LveModel::Vertex const &vertex) const {
			size_t seed = 0;
			lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace lve {
	LveModel::LveModel(LveDevice& device, const Builder& builder) 
		: _lveDevice{ device }
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	LveModel::~LveModel() {
		vkDestroyBuffer(_lveDevice.device(), _vertexBuffer, nullptr);
		vkFreeMemory(_lveDevice.device(), _vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(_lveDevice.device(), _indexBuffer, nullptr);
			vkFreeMemory(_lveDevice.device(), _indexBufferMemory, nullptr);
		}
	}

	std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice& device, const std::string& filepath)
	{
		Builder builder;
		builder.loadModel(filepath);
		std::cout << "Vertex count: " << builder.vertices.size() << "\n";
		return std::make_unique<LveModel>(device, builder);
	}


	void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(_vertexCount >= 3 && "Veretx count must be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * _vertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		_lveDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(_lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), bufferSize);
		vkUnmapMemory(_lveDevice.device(), stagingBufferMemory);

		_lveDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_vertexBuffer,
			_vertexBufferMemory);

		_lveDevice.copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

		vkDestroyBuffer(_lveDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(_lveDevice.device(), stagingBufferMemory, nullptr);
	}

	void LveModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		_indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = (_indexCount > 0);
		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * _indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		_lveDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(_lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), bufferSize);
		vkUnmapMemory(_lveDevice.device(), stagingBufferMemory);

		_lveDevice.createBuffer(bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_indexBuffer,
			_indexBufferMemory);

		_lveDevice.copyBuffer(stagingBuffer, _indexBuffer, bufferSize);

		vkDestroyBuffer(_lveDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(_lveDevice.device(), stagingBufferMemory, nullptr);
	}


	void LveModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void LveModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, _vertexCount, 1, 0, 0);
		}
	}

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}

	void LveModel::Builder::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};
				if (index.vertex_index >= 0) {
					vertex.position = { 
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					auto colorIndex = 3 * index.vertex_index + 2;
					if (colorIndex < attrib.colors.size()) {
						vertex.color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex - 0]
						};
					}
					else {
						vertex.color = { 1.f, 1.f, 1.f };
					}
				}
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);

			}
		}

	}
}